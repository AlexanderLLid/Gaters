#include "GatersBuildingEvaluator.h"

#include "GatersTerrainNavigation.h"

namespace
{
void AddIssue(
	FGatersBuildingEvaluation& Evaluation,
	const TCHAR* RuleId,
	const FString& SubjectId,
	const TCHAR* Message)
{
	Evaluation.Issues.Add({RuleId, SubjectId, Message});
}

bool IsFiniteTransform(const FTransform& Transform)
{
	const FVector Location = Transform.GetLocation();
	const FVector Scale = Transform.GetScale3D();
	return FMath::IsFinite(Location.X) && FMath::IsFinite(Location.Y) && FMath::IsFinite(Location.Z)
		&& FMath::IsFinite(Scale.X) && FMath::IsFinite(Scale.Y) && FMath::IsFinite(Scale.Z)
		&& Scale.X > 0.f && Scale.Y > 0.f && Scale.Z > 0.f;
}

bool HasValidSources(const TArray<FString>& SourceIds)
{
	return !SourceIds.IsEmpty() && !SourceIds.ContainsByPredicate(
		[](const FString& SourceId) { return SourceId.IsEmpty(); });
}

bool IsFinitePositiveExtent(const FVector& Extent)
{
	return FMath::IsFinite(Extent.X) && FMath::IsFinite(Extent.Y)
		&& FMath::IsFinite(Extent.Z) && Extent.X > 0.f && Extent.Y > 0.f
		&& Extent.Z > 0.f;
}
}

FString FGatersBuildingEvaluation::Summary() const
{
	return FString::Printf(TEXT("valid=%s modules=%d unique=%d entrances=%d issues=%d"),
		IsValid() ? TEXT("yes") : TEXT("no"), ModuleCount, UniqueModuleCount,
		EntranceCount, Issues.Num());
}

FGatersBuildingEvaluation FGatersBuildingEvaluator::Evaluate(
	const FGatersTerrainSemanticField& Field,
	const FGatersBuildingAssembly& Assembly)
{
	FGatersBuildingEvaluation Result;
	Result.ModuleCount = Assembly.Modules.Num();
	if (!Assembly.bGenerated)
	{
		AddIssue(Result, TEXT("building.plan.invalid"), Assembly.BuildingId,
			TEXT("building generator did not produce a complete assembly"));
	}
	if (Assembly.GroundCell.X < 0 || Assembly.GroundCell.X >= Field.CellsPerAxis
		|| Assembly.GroundCell.Y < 0 || Assembly.GroundCell.Y >= Field.CellsPerAxis
		|| !FGatersTerrainNavigation::IsWalkable(
			Field.At(Assembly.GroundCell.X, Assembly.GroundCell.Y).Type))
	{
		AddIssue(Result, TEXT("building.terrain.blocked"), Assembly.BuildingId,
			TEXT("building anchor is not on walkable terrain"));
	}

	TSet<FString> Ids;
	TSet<int32> SupportedFloors{0};
	for (const FGatersBuildingModule& Module : Assembly.Modules)
	{
		if (Module.Id.IsEmpty() || Ids.Contains(Module.Id))
		{
			AddIssue(Result, TEXT("building.identity.duplicate"), Module.Id,
				TEXT("module IDs must be non-empty and unique"));
		}
		Ids.Add(Module.Id);
		Result.EntranceCount += Module.Kind == EGatersBuildingModuleKind::DoorWall ? 1 : 0;
		if (Module.Kind == EGatersBuildingModuleKind::Floor)
		{
			SupportedFloors.Add(Module.FloorIndex);
		}
		if (!IsFiniteTransform(Module.Transform))
		{
			AddIssue(Result, TEXT("building.transform.invalid"), Module.Id,
				TEXT("module transform must be finite with positive scale"));
		}
		if (Module.FloorIndex < 0 || Module.FloorIndex > Assembly.FloorCount)
		{
			AddIssue(Result, TEXT("building.floor.invalid"), Module.Id,
				TEXT("module floor is outside the assembly"));
		}
	}
	Result.UniqueModuleCount = Ids.Num();
	// ponytail: assemblies are small; replace the quadratic exact-overlap check only if
	// measured player-built structures make it material.
	for (int32 A = 0; A < Assembly.Modules.Num(); ++A)
	{
		for (int32 B = A + 1; B < Assembly.Modules.Num(); ++B)
		{
			if (Assembly.Modules[A].Transform.Equals(Assembly.Modules[B].Transform, 0.01f))
			{
				AddIssue(Result, TEXT("building.module.overlap"), Assembly.Modules[B].Id,
					TEXT("two modules occupy the same transform"));
			}
		}
	}
	if (Assembly.Count(EGatersBuildingModuleKind::Foundation) != 1)
	{
		AddIssue(Result, TEXT("building.foundation.missing"), Assembly.BuildingId,
			TEXT("assembly requires exactly one foundation"));
	}
	if (Result.EntranceCount != 1)
	{
		AddIssue(Result, TEXT("building.entrance.missing"), Assembly.BuildingId,
			TEXT("assembly requires exactly one ground-floor entrance"));
	}
	if (Assembly.UsableSpaces.Num() != Assembly.FloorCount)
	{
		AddIssue(Result, TEXT("building.space.missing"), Assembly.BuildingId,
			TEXT("assembly requires one usable volume per floor"));
	}
	TSet<FString> SpaceIds;
	TSet<int32> SpaceFloors;
	for (const FGatersBuildingUsableSpace& Space : Assembly.UsableSpaces)
	{
		if (Space.Id.IsEmpty() || SpaceIds.Contains(Space.Id)
			|| SpaceFloors.Contains(Space.FloorIndex)
			|| Space.FloorIndex < 0 || Space.FloorIndex >= Assembly.FloorCount
			|| !FMath::IsFinite(Space.Center.X) || !FMath::IsFinite(Space.Center.Y)
			|| !FMath::IsFinite(Space.Center.Z) || !IsFinitePositiveExtent(Space.Extent)
			|| !HasValidSources(Space.SourceIds))
		{
			AddIssue(Result, TEXT("building.space.invalid"), Space.Id,
				TEXT("usable volume identity, floor, dimensions, or provenance is invalid"));
		}
		SpaceIds.Add(Space.Id);
		SpaceFloors.Add(Space.FloorIndex);
	}
	if (Assembly.Openings.Num() != Result.EntranceCount)
	{
		AddIssue(Result, TEXT("building.opening.missing"), Assembly.BuildingId,
			TEXT("each semantic entrance requires one physical opening"));
	}
	TSet<FString> OpeningIds;
	for (const FGatersBuildingOpening& Opening : Assembly.Openings)
	{
		const FGatersBuildingModule* SourceModule = Assembly.Modules.FindByPredicate(
			[&Opening](const FGatersBuildingModule& Module)
			{
				return Module.Id == Opening.SourceModuleId;
			});
		if (Opening.Id.IsEmpty() || OpeningIds.Contains(Opening.Id)
			|| !SourceModule || SourceModule->Kind != EGatersBuildingModuleKind::DoorWall
			|| Opening.FloorIndex != 0 || !IsFiniteTransform(Opening.Transform)
			|| !FMath::IsFinite(Opening.Width) || Opening.Width <= 0.f
			|| !FMath::IsFinite(Opening.Headroom) || Opening.Headroom <= 0.f
			|| !FMath::IsFinite(Opening.Depth) || Opening.Depth <= 0.f
			|| !HasValidSources(Opening.SourceIds)
			|| !Opening.SourceIds.Contains(Opening.SourceModuleId))
		{
			AddIssue(Result, TEXT("building.opening.invalid"), Opening.Id,
				TEXT("opening source, transform, clearance, or provenance is invalid"));
		}
		OpeningIds.Add(Opening.Id);
	}
	if (Assembly.Count(EGatersBuildingModuleKind::Roof) != 1)
	{
		AddIssue(Result, TEXT("building.roof.missing"), Assembly.BuildingId,
			TEXT("assembly requires exactly one roof"));
	}
	for (int32 Floor = 1; Floor < Assembly.FloorCount; ++Floor)
	{
		if (!SupportedFloors.Contains(Floor))
		{
			AddIssue(Result, TEXT("building.support.missing"), Assembly.BuildingId,
				TEXT("upper storey has no supporting floor module"));
		}
	}
	return Result;
}
