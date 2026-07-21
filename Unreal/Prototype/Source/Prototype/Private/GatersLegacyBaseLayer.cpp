#include "GatersLegacyBaseLayer.h"

namespace
{
constexpr int32 ContractVersion = 1;
constexpr int32 GeneratorVersion = 1;

void AddDiagnostic(
	FGatersLegacyBaseLayerResult& Result,
	const TCHAR* RuleId,
	const FString& Message)
{
	Result.Diagnostics.Add(FString::Printf(TEXT("%s: %s"), RuleId, *Message));
}

bool ValidateModule(
	const FGatersLegacyBaseModuleDefinition& Module,
	const bool bRequired,
	const FString& Name,
	FGatersLegacyBaseLayerResult& Result,
	TSet<FString>& ContentKeys)
{
	if (!Module.bAvailable)
	{
		if (bRequired)
		{
			AddDiagnostic(Result, TEXT("legacy-base.input.module"),
				FString::Printf(TEXT("%s is required"), *Name));
			return false;
		}
		return true;
	}

	TArray<FString> Errors;
	if (!Module.Contract.Validate(Errors))
	{
		for (const FString& Error : Errors)
		{
			AddDiagnostic(Result, TEXT("legacy-base.input.module"),
				FString::Printf(TEXT("%s: %s"), *Name, *Error));
		}
		return false;
	}
	if (ContentKeys.Contains(Module.Contract.ContentKey))
	{
		AddDiagnostic(Result, TEXT("legacy-base.input.duplicate-content"),
			FString::Printf(TEXT("%s repeats %s"), *Name, *Module.Contract.ContentKey));
		return false;
	}
	ContentKeys.Add(Module.Contract.ContentKey);
	return true;
}

bool ValidateInput(
	const FGatersLegacyBaseLayerInput& Input,
	FGatersLegacyBaseLayerResult& Result)
{
	if (Input.Version != ContractVersion)
	{
		AddDiagnostic(Result, TEXT("legacy-base.input.version"),
			FString::Printf(TEXT("expected %d, received %d"), ContractVersion, Input.Version));
	}
	if (!FMath::IsFinite(Input.BaseCenter.X) || !FMath::IsFinite(Input.BaseCenter.Y) ||
		!FMath::IsFinite(Input.CellSize) || Input.CellSize <= 0.f ||
		!FMath::IsFinite(Input.MaxFoundationDrop) || Input.MaxFoundationDrop < 0.f)
	{
		AddDiagnostic(Result, TEXT("legacy-base.input.dimensions"),
			TEXT("base center, cell size, and foundation drop must be finite and usable"));
	}
	if (Input.SourceIds.IsEmpty() || Input.SourceIds.ContainsByPredicate(
		[](const FString& SourceId) { return SourceId.IsEmpty(); }))
	{
		AddDiagnostic(Result, TEXT("legacy-base.input.provenance"),
			TEXT("at least one non-empty source ID is required"));
	}
	if (Input.Tiers.Num() != 3)
	{
		AddDiagnostic(Result, TEXT("legacy-base.input.tiers"),
			TEXT("exactly three ordered material tiers are required"));
	}

	TSet<FString> TierIds;
	TSet<FString> ContentKeys;
	for (int32 TierIndex = 0; TierIndex < Input.Tiers.Num(); ++TierIndex)
	{
		const FGatersLegacyBaseTierDefinition& Tier = Input.Tiers[TierIndex];
		if (Tier.Id.IsEmpty() || TierIds.Contains(Tier.Id))
		{
			AddDiagnostic(Result, TEXT("legacy-base.input.tiers"),
				FString::Printf(TEXT("tier %d requires a unique non-empty ID"), TierIndex));
		}
		TierIds.Add(Tier.Id);
		const FString Prefix = Tier.Id.IsEmpty()
			? FString::Printf(TEXT("tier:%d"), TierIndex)
			: Tier.Id;
		ValidateModule(Tier.Foundation, true, Prefix + TEXT(".foundation"), Result, ContentKeys);
		ValidateModule(Tier.Wall, true, Prefix + TEXT(".wall"), Result, ContentKeys);
		ValidateModule(Tier.DoorFrame, true, Prefix + TEXT(".doorframe"), Result, ContentKeys);
		ValidateModule(Tier.Window, false, Prefix + TEXT(".window"), Result, ContentKeys);
		ValidateModule(Tier.Ceiling, true, Prefix + TEXT(".ceiling"), Result, ContentKeys);
		ValidateModule(Tier.Fence, false, Prefix + TEXT(".fence"), Result, ContentKeys);
	}
	ValidateModule(Input.Door, true, TEXT("door"), Result, ContentKeys);
	return Result.Diagnostics.IsEmpty();
}

FBox BoundsOf(const FGatersLegacyBaseModuleDefinition& Module)
{
	return FBox(
		Module.Contract.BoundsCenter - Module.Contract.BoundsExtent,
		Module.Contract.BoundsCenter + Module.Contract.BoundsExtent);
}
}

bool FGatersLegacyBaseLayerResult::IsValid() const
{
	return Version == ContractVersion && GeneratorVersion == ::GeneratorVersion &&
		Diagnostics.IsEmpty() && !Pieces.IsEmpty();
}

FGatersLegacyBaseLayerResult FGatersLegacyBaseLayer::Generate(
	const FGatersLegacyBaseLayerInput& Input,
	TFunctionRef<float(const FVector2D&)> HeightAt)
{
	FGatersLegacyBaseLayerResult Result;
	Result.Version = ContractVersion;
	Result.GeneratorVersion = GeneratorVersion;
	if (!ValidateInput(Input, Result))
	{
		return Result;
	}

	FRandomStream Random(Input.RandomState);
	TSet<FString> UsedContentKeys;
	bool bTerrainInvalid = false;
	bool bOutputInvalid = false;
	auto SampleHeight = [&](const FVector2D& Point)
	{
		const float Height = HeightAt(Point);
		if (!FMath::IsFinite(Height))
		{
			bTerrainInvalid = true;
			return 0.f;
		}
		return Height;
	};
	auto AddPiece = [&](const FGatersLegacyBaseModuleDefinition& Module,
		const FVector& Position, const float Yaw, const FVector& Scale = FVector::OneVector)
	{
		FGatersLegacyBasePieceRecipe Piece;
		Piece.Id = FString::Printf(TEXT("piece:%d"), Result.Pieces.Num());
		Piece.Transform = FTransform(FRotator(0.f, Yaw, 0.f), Position, Scale);
		if (!Piece.Transform.IsValid() || Scale.X <= 0.f || Scale.Y <= 0.f || Scale.Z <= 0.f)
		{
			bOutputInvalid = true;
			return;
		}
		Piece.ContentKey = Module.Contract.ContentKey;
		Piece.SourceIds = Input.SourceIds;
		Piece.SourceIds.AddUnique(Module.Contract.AssetId);
		Result.Pieces.Add(MoveTemp(Piece));
		if (!UsedContentKeys.Contains(Module.Contract.ContentKey))
		{
			UsedContentKeys.Add(Module.Contract.ContentKey);
			Result.ContentRequirements.Add(Module.Contract);
		}
	};

	auto AddBuilding = [&](const FVector2D& Center, const int32 Width, const int32 Depth,
		const int32 Stories, const int32 TierIndex)
	{
		const FGatersLegacyBaseTierDefinition& Tier = Input.Tiers[TierIndex];
		const FBox FoundationBounds = BoundsOf(Tier.Foundation);
		const FBox WallBounds = BoundsOf(Tier.Wall);
		const FBox CeilingBounds = BoundsOf(Tier.Ceiling);
		const float FoundationHeight = FoundationBounds.GetSize().Z;
		const float WallHeight = WallBounds.GetSize().Z;
		const float HalfX = Width * Input.CellSize * 0.5f;
		const float HalfY = Depth * Input.CellSize * 0.5f;
		float MinHeight = TNumericLimits<float>::Max();
		float MaxHeight = -TNumericLimits<float>::Max();
		const FVector2D FootprintPoints[] = {
			Center,
			Center + FVector2D(HalfX, HalfY),
			Center + FVector2D(-HalfX, HalfY),
			Center + FVector2D(HalfX, -HalfY),
			Center + FVector2D(-HalfX, -HalfY)};
		for (const FVector2D& Point : FootprintPoints)
		{
			const float Height = SampleHeight(Point);
			MinHeight = FMath::Min(MinHeight, Height);
			MaxHeight = FMath::Max(MaxHeight, Height);
		}
		if (bTerrainInvalid || MaxHeight - MinHeight > Input.MaxFoundationDrop)
		{
			return false;
		}

		const float FoundationTop = MaxHeight + FoundationHeight;
		for (int32 XIndex = 0; XIndex < Width; ++XIndex)
		{
			for (int32 YIndex = 0; YIndex < Depth; ++YIndex)
			{
				const float X = Center.X + (XIndex - (Width - 1) * 0.5f) * Input.CellSize;
				const float Y = Center.Y + (YIndex - (Depth - 1) * 0.5f) * Input.CellSize;
				const float HalfCell = Input.CellSize * 0.5f;
				const float GroundHere = FMath::Min(
					FMath::Min(SampleHeight(FVector2D(X - HalfCell, Y - HalfCell)),
						SampleHeight(FVector2D(X + HalfCell, Y - HalfCell))),
					FMath::Min(SampleHeight(FVector2D(X - HalfCell, Y + HalfCell)),
						SampleHeight(FVector2D(X + HalfCell, Y + HalfCell))));
				const float BottomZ = GroundHere - 60.f;
				const float ScaleZ = (FoundationTop - BottomZ) / FoundationHeight;
				const float PivotZ = FoundationTop - FoundationBounds.Max.Z * ScaleZ;
				AddPiece(Tier.Foundation, FVector(X, Y, PivotZ), 0.f,
					FVector(1.f, 1.f, ScaleZ));
			}
		}

		const int32 DoorSide = Random.RandRange(0, 3);
		const int32 DoorSlot = Random.RandRange(0, (DoorSide < 2 ? Width : Depth) - 1);
		for (int32 Story = 0; Story < Stories; ++Story)
		{
			const float BandBottom = FoundationTop + Story * WallHeight;
			const float WallPivotZ = BandBottom - WallBounds.Min.Z;
			auto AddWallSlot = [&](const FVector2D& Position, const float Yaw, const bool bDoorHere)
			{
				const float WindowRoll = Random.FRandRange(0.f, 1.f);
				const FGatersLegacyBaseModuleDefinition* Module = &Tier.Wall;
				if (bDoorHere)
				{
					Module = &Tier.DoorFrame;
				}
				else if (WindowRoll < 0.22f && Tier.Window.bAvailable)
				{
					Module = &Tier.Window;
				}
				AddPiece(*Module, FVector(Position.X, Position.Y, WallPivotZ), Yaw);
				if (bDoorHere)
				{
					AddPiece(Input.Door, FVector(Position.X, Position.Y, WallPivotZ), Yaw);
				}
			};
			for (int32 XIndex = 0; XIndex < Width; ++XIndex)
			{
				const float X = Center.X + (XIndex - (Width - 1) * 0.5f) * Input.CellSize;
				AddWallSlot(FVector2D(X, Center.Y - HalfY), 0.f,
					Story == 0 && DoorSide == 0 && DoorSlot == XIndex);
				AddWallSlot(FVector2D(X, Center.Y + HalfY), 0.f,
					Story == 0 && DoorSide == 1 && DoorSlot == XIndex);
			}
			for (int32 YIndex = 0; YIndex < Depth; ++YIndex)
			{
				const float Y = Center.Y + (YIndex - (Depth - 1) * 0.5f) * Input.CellSize;
				AddWallSlot(FVector2D(Center.X - HalfX, Y), 90.f,
					Story == 0 && DoorSide == 2 && DoorSlot == YIndex);
				AddWallSlot(FVector2D(Center.X + HalfX, Y), 90.f,
					Story == 0 && DoorSide == 3 && DoorSlot == YIndex);
			}
		}

		const float RoofPivotZ = FoundationTop + Stories * WallHeight - CeilingBounds.Min.Z;
		for (int32 XIndex = 0; XIndex < Width; ++XIndex)
		{
			for (int32 YIndex = 0; YIndex < Depth; ++YIndex)
			{
				AddPiece(Tier.Ceiling,
					FVector(Center.X + (XIndex - (Width - 1) * 0.5f) * Input.CellSize,
						Center.Y + (YIndex - (Depth - 1) * 0.5f) * Input.CellSize,
						RoofPivotZ), 0.f);
			}
		}
		return true;
	};

	const int32 ArchetypeIndex = Random.RandRange(0, 2);
	Result.Archetype = static_cast<EGatersLegacyBaseArchetype>(ArchetypeIndex);
	const int32 MainTierIndex = Random.RandRange(0, 2);
	Result.MainTierId = Input.Tiers[MainTierIndex].Id;
	Result.MainWidth = Random.RandRange(2, 4);
	Result.MainDepth = Random.RandRange(2, 4);
	Result.MainStories = Random.FRandRange(0.f, 1.f) < 0.4f ? 2 : 1;
	if (Result.Archetype == EGatersLegacyBaseArchetype::Tower)
	{
		Result.MainWidth = Random.RandRange(1, 2);
		Result.MainDepth = Result.MainWidth;
		Result.MainStories = Random.RandRange(3, 4);
	}
	if (!AddBuilding(Input.BaseCenter, Result.MainWidth, Result.MainDepth,
		Result.MainStories, MainTierIndex))
	{
		AddDiagnostic(Result,
			bTerrainInvalid ? TEXT("legacy-base.terrain.non-finite") : TEXT("legacy-base.fit.foundation-drop"),
			TEXT("the main building footprint does not fit the supplied terrain"));
		Result.Pieces.Reset();
		Result.ContentRequirements.Reset();
		return Result;
	}
	Result.BuildingCount = 1;

	if (Result.Archetype == EGatersLegacyBaseArchetype::Compound)
	{
		const int32 ExtraBuildings = Random.RandRange(1, 2);
		for (int32 ExtraIndex = 0; ExtraIndex < ExtraBuildings; ++ExtraIndex)
		{
			const float Angle = Random.FRandRange(0.f, 360.f);
			const float Distance = Random.FRandRange(1100.f, 1600.f);
			const FVector2D Center = Input.BaseCenter + FVector2D(
				Distance * FMath::Cos(FMath::DegreesToRadians(Angle)),
				Distance * FMath::Sin(FMath::DegreesToRadians(Angle)));
			const int32 ShedTier = Random.RandRange(0, MainTierIndex);
			if (AddBuilding(Center, Random.RandRange(1, 2), Random.RandRange(1, 3), 1, ShedTier))
			{
				++Result.BuildingCount;
			}
		}

		const FGatersLegacyBaseModuleDefinition& Fence = Input.Tiers[MainTierIndex].Fence;
		if (Fence.bAvailable)
		{
			const float FenceMinZ = BoundsOf(Fence).Min.Z;
			const float FenceRadius = 2100.f;
			const int32 Sections = FMath::FloorToInt(2.f * FenceRadius / Input.CellSize);
			const int32 GapSide = Random.RandRange(0, 3);
			for (int32 Side = 0; Side < 4; ++Side)
			{
				for (int32 Section = 0; Section < Sections; ++Section)
				{
					if (Side == GapSide && Section >= Sections / 2 - 1 &&
						Section <= Sections / 2 + 1)
					{
						continue;
					}
					const float Along = (Section - (Sections - 1) * 0.5f) * Input.CellSize;
					FVector2D Position;
					float Yaw = 0.f;
					switch (Side)
					{
					case 0: Position = Input.BaseCenter + FVector2D(Along, -FenceRadius); break;
					case 1: Position = Input.BaseCenter + FVector2D(Along, FenceRadius); break;
					case 2:
						Position = Input.BaseCenter + FVector2D(-FenceRadius, Along);
						Yaw = 90.f;
						break;
					default:
						Position = Input.BaseCenter + FVector2D(FenceRadius, Along);
						Yaw = 90.f;
						break;
					}
					AddPiece(Fence,
						FVector(Position.X, Position.Y,
							SampleHeight(Position) - 20.f - FenceMinZ), Yaw);
				}
			}
		}
	}

	if (bTerrainInvalid || bOutputInvalid)
	{
		AddDiagnostic(Result,
			bTerrainInvalid ? TEXT("legacy-base.terrain.non-finite") : TEXT("legacy-base.terrain.range"),
			bTerrainInvalid
				? TEXT("terrain query returned a non-finite height")
				: TEXT("terrain evidence could not produce finite positive physical transforms"));
		Result.Pieces.Reset();
		Result.ContentRequirements.Reset();
	}
	return Result;
}
