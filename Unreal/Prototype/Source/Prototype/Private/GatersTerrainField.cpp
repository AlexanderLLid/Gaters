#include "GatersTerrainField.h"

namespace
{
constexpr int32 CubeCorners[8][3] = {
	{ 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 },
	{ 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 }
};

// Six tetrahedra sharing the cube's 0-6 diagonal. Neighboring cubes choose the same
// diagonal on their shared face, so independently rebuilt cells do not crack.
constexpr int32 CubeTetrahedra[6][4] = {
	{ 0, 5, 1, 6 }, { 0, 1, 2, 6 }, { 0, 2, 3, 6 },
	{ 0, 3, 7, 6 }, { 0, 7, 4, 6 }, { 0, 4, 5, 6 }
};

constexpr int32 TetraEdges[6][2] = {
	{ 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 }
};
}

FGatersTerrainField::FGatersTerrainField(int32 InCellsPerAxis, float InCellSize)
	: CellsPerAxis(FMath::Max(1, InCellsPerAxis))
	, SamplesPerAxis(CellsPerAxis + 1)
	, CellSize(FMath::Max(1.f, InCellSize))
{
	Density.SetNumZeroed(SamplesPerAxis * SamplesPerAxis * SamplesPerAxis);
}

int32 FGatersTerrainField::SampleIndex(int32 X, int32 Y, int32 Z) const
{
	return (Z * SamplesPerAxis + Y) * SamplesPerAxis + X;
}

FVector3d FGatersTerrainField::SamplePosition(int32 X, int32 Y, int32 Z) const
{
	const double HalfExtent = CellsPerAxis * CellSize * 0.5;
	return FVector3d(
		X * CellSize - HalfExtent,
		Y * CellSize - HalfExtent,
		Z * CellSize - HalfExtent);
}

void FGatersTerrainField::Generate(int32 Seed)
{
	FRandomStream Random(Seed);
	const double PhaseX = Random.FRandRange(-PI, PI);
	const double PhaseY = Random.FRandRange(-PI, PI);
	const double Extent = CellsPerAxis * CellSize;

	for (int32 Z = 0; Z < SamplesPerAxis; ++Z)
	{
		for (int32 Y = 0; Y < SamplesPerAxis; ++Y)
		{
			for (int32 X = 0; X < SamplesPerAxis; ++X)
			{
				const FVector3d Position = SamplePosition(X, Y, Z);
				const double Height =
					FMath::Sin(Position.X / Extent * 2.0 * PI + PhaseX) * Extent * 0.10 +
					FMath::Cos(Position.Y / Extent * 2.0 * PI + PhaseY) * Extent * 0.07;
				Density[SampleIndex(X, Y, Z)] = static_cast<float>(Height - Position.Z);
			}
		}
	}
}

void FGatersTerrainField::Apply(const FGatersTerrainEdit& Edit)
{
	if (Edit.Radius <= 0.f)
	{
		return;
	}
	for (int32 Z = 0; Z < SamplesPerAxis; ++Z)
	{
		for (int32 Y = 0; Y < SamplesPerAxis; ++Y)
		{
			for (int32 X = 0; X < SamplesPerAxis; ++X)
			{
				float& Value = Density[SampleIndex(X, Y, Z)];
				const float OutsideSphere = static_cast<float>(
					(SamplePosition(X, Y, Z) - FVector3d(Edit.Center)).Length()) - Edit.Radius;
				Value = FMath::Min(Value, OutsideSphere);
			}
		}
	}
}

bool FGatersTerrainField::SurfaceHeightAt(const FVector2D& Point, float& OutHeight) const
{
	const float HalfExtent = CellsPerAxis * CellSize * 0.5f;
	const float GridX = (Point.X + HalfExtent) / CellSize;
	const float GridY = (Point.Y + HalfExtent) / CellSize;
	if (GridX < 0.f || GridY < 0.f || GridX > CellsPerAxis || GridY > CellsPerAxis)
	{
		return false;
	}

	const int32 X0 = FMath::Clamp(FMath::FloorToInt(GridX), 0, SamplesPerAxis - 2);
	const int32 Y0 = FMath::Clamp(FMath::FloorToInt(GridY), 0, SamplesPerAxis - 2);
	const float AlphaX = FMath::Clamp(GridX - X0, 0.f, 1.f);
	const float AlphaY = FMath::Clamp(GridY - Y0, 0.f, 1.f);
	auto DensityAtZ = [this, X0, Y0, AlphaX, AlphaY](int32 Z)
	{
		const float A = FMath::Lerp(
			Density[SampleIndex(X0, Y0, Z)], Density[SampleIndex(X0 + 1, Y0, Z)], AlphaX);
		const float B = FMath::Lerp(
			Density[SampleIndex(X0, Y0 + 1, Z)],
			Density[SampleIndex(X0 + 1, Y0 + 1, Z)], AlphaX);
		return FMath::Lerp(A, B, AlphaY);
	};

	for (int32 Z = 0; Z < SamplesPerAxis - 1; ++Z)
	{
		const float Below = DensityAtZ(Z);
		const float Above = DensityAtZ(Z + 1);
		if (Below > 0.f && Above <= 0.f)
		{
			const float AlphaZ = Below / (Below - Above);
			OutHeight = static_cast<float>(SamplePosition(0, 0, Z).Z) + AlphaZ * CellSize;
			return true;
		}
	}
	return false;
}

float FGatersTerrainField::FootprintDrop(const FVector2D& Center, float Radius) const
{
	float MinHeight = TNumericLimits<float>::Max();
	float MaxHeight = TNumericLimits<float>::Lowest();
	for (int32 Sample = 0; Sample <= 16; ++Sample)
	{
		const float Angle = Sample * 2.f * PI / 16.f;
		const FVector2D Point = Sample == 16
			? Center
			: Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
		float Height = 0.f;
		if (!SurfaceHeightAt(Point, Height))
		{
			return TNumericLimits<float>::Max();
		}
		MinHeight = FMath::Min(MinHeight, Height);
		MaxHeight = FMath::Max(MaxHeight, Height);
	}
	return MaxHeight - MinHeight;
}

bool FGatersTerrainField::TryFitContact(
	const FGatersTerrainContact& Contact,
	FGatersTerrainContactResult& OutResult)
{
	OutResult = FGatersTerrainContactResult();
	if (Contact.Radius <= 0.f)
	{
		return false;
	}

	float HeightSum = 0.f;
	float Heights[17];
	for (int32 Sample = 0; Sample <= 16; ++Sample)
	{
		const float Angle = Sample * 2.f * PI / 16.f;
		const FVector2D Point = Sample == 16
			? Contact.Center
			: Contact.Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Contact.Radius;
		if (!SurfaceHeightAt(Point, Heights[Sample]))
		{
			OutResult.Status = EGatersTerrainContactStatus::OutsideField;
			return false;
		}
		HeightSum += Heights[Sample];
	}

	const float TargetHeight = HeightSum / 17.f;
	for (const float Height : Heights)
	{
		OutResult.RequiredTerrainChange = FMath::Max(
			OutResult.RequiredTerrainChange, FMath::Abs(Height - TargetHeight));
	}
	OutResult.Placement = FVector(Contact.Center, TargetHeight);
	if (OutResult.RequiredTerrainChange > FMath::Max(Contact.MaxTerrainChange, 0.f))
	{
		OutResult.Status = EGatersTerrainContactStatus::TooUneven;
		return false;
	}

	const float OuterRadius = Contact.Radius + FMath::Max(Contact.BlendRadius, 0.f);
	for (int32 Y = 0; Y < SamplesPerAxis; ++Y)
	{
		for (int32 X = 0; X < SamplesPerAxis; ++X)
		{
			const FVector3d XYPosition = SamplePosition(X, Y, 0);
			const float Distance = FVector2D::Distance(
				Contact.Center, FVector2D(XYPosition.X, XYPosition.Y));
			if (Distance >= OuterRadius)
			{
				continue;
			}
			const float Weight = Distance <= Contact.Radius || OuterRadius <= Contact.Radius
				? 1.f
				: 1.f - FMath::SmoothStep(Contact.Radius, OuterRadius, Distance);
			for (int32 Z = 0; Z < SamplesPerAxis; ++Z)
			{
				float& Value = Density[SampleIndex(X, Y, Z)];
				const float Desired = TargetHeight - static_cast<float>(SamplePosition(X, Y, Z).Z);
				Value = FMath::Lerp(Value, Desired, Weight);
			}
		}
	}
	OutResult.Status = EGatersTerrainContactStatus::Accepted;
	return true;
}

int64 FGatersTerrainField::CellId(int32 X, int32 Y, int32 Z) const
{
	check(X >= 0 && X < CellsPerAxis);
	check(Y >= 0 && Y < CellsPerAxis);
	check(Z >= 0 && Z < CellsPerAxis);
	return (static_cast<int64>(Z) * CellsPerAxis + Y) * CellsPerAxis + X;
}

int32 FGatersTerrainField::SolidSampleCount() const
{
	int32 Count = 0;
	for (const float Value : Density)
	{
		Count += Value > 0.f ? 1 : 0;
	}
	return Count;
}

uint64 FGatersTerrainField::DensityChecksum() const
{
	uint64 Hash = 1469598103934665603ull;
	for (const float Value : Density)
	{
		uint32 Bits = 0;
		FMemory::Memcpy(&Bits, &Value, sizeof(float));
		Hash ^= Bits;
		Hash *= 1099511628211ull;
	}
	return Hash;
}

void FGatersTerrainField::BuildMesh(UE::Geometry::FDynamicMesh3& OutMesh) const
{
	OutMesh.Clear();
	TMap<FIntVector, int32> VertexIds;

	auto AddVertex = [&OutMesh, &VertexIds](const FVector3d& Position)
	{
		const FIntVector Key(
			FMath::RoundToInt(Position.X * 100.0),
			FMath::RoundToInt(Position.Y * 100.0),
			FMath::RoundToInt(Position.Z * 100.0));
		if (const int32* Existing = VertexIds.Find(Key))
		{
			return *Existing;
		}
		const int32 Id = OutMesh.AppendVertex(Position);
		VertexIds.Add(Key, Id);
		return Id;
	};

	for (int32 Z = 0; Z < CellsPerAxis; ++Z)
	{
		for (int32 Y = 0; Y < CellsPerAxis; ++Y)
		{
			for (int32 X = 0; X < CellsPerAxis; ++X)
			{
				FVector3d Positions[8];
				float Values[8];
				bool bAnySolid = false;
				bool bAnyEmpty = false;
				for (int32 Corner = 0; Corner < 8; ++Corner)
				{
					const int32 SX = X + CubeCorners[Corner][0];
					const int32 SY = Y + CubeCorners[Corner][1];
					const int32 SZ = Z + CubeCorners[Corner][2];
					Positions[Corner] = SamplePosition(SX, SY, SZ);
					Values[Corner] = Density[SampleIndex(SX, SY, SZ)];
					bAnySolid |= Values[Corner] > 0.f;
					bAnyEmpty |= Values[Corner] <= 0.f;
				}
				if (!bAnySolid || !bAnyEmpty)
				{
					continue;
				}

				for (const auto& Tetra : CubeTetrahedra)
				{
					TArray<FVector3d, TInlineAllocator<4>> Polygon;
					FVector3d InsideCenter = FVector3d::Zero();
					FVector3d OutsideCenter = FVector3d::Zero();
					int32 InsideCount = 0;
					int32 OutsideCount = 0;

					for (int32 Local = 0; Local < 4; ++Local)
					{
						const int32 Corner = Tetra[Local];
						if (Values[Corner] > 0.f)
						{
							InsideCenter += Positions[Corner];
							++InsideCount;
						}
						else
						{
							OutsideCenter += Positions[Corner];
							++OutsideCount;
						}
					}
					if (InsideCount == 0 || OutsideCount == 0)
					{
						continue;
					}

					for (const auto& Edge : TetraEdges)
					{
						const int32 A = Tetra[Edge[0]];
						const int32 B = Tetra[Edge[1]];
						if ((Values[A] > 0.f) == (Values[B] > 0.f))
						{
							continue;
						}
						const double Alpha = Values[A] / static_cast<double>(Values[A] - Values[B]);
						const FVector3d Point = FMath::Lerp(Positions[A], Positions[B], Alpha);
						bool bDuplicate = false;
						for (const FVector3d& Existing : Polygon)
						{
							bDuplicate |= FVector3d::DistSquared(Existing, Point) < 0.0001;
						}
						if (!bDuplicate)
						{
							Polygon.Add(Point);
						}
					}
					if (Polygon.Num() < 3)
					{
						continue;
					}

					InsideCenter /= InsideCount;
					OutsideCenter /= OutsideCount;
					FVector3d Outward = (OutsideCenter - InsideCenter).GetSafeNormal();
					FVector3d Center = FVector3d::Zero();
					for (const FVector3d& Point : Polygon)
					{
						Center += Point;
					}
					Center /= Polygon.Num();
					const FVector3d U = (Polygon[0] - Center).GetSafeNormal();
					const FVector3d V = Outward.Cross(U).GetSafeNormal();
					Polygon.Sort([Center, U, V](const FVector3d& A, const FVector3d& B)
					{
						const FVector3d DA = A - Center;
						const FVector3d DB = B - Center;
						return FMath::Atan2(DA.Dot(V), DA.Dot(U)) <
							FMath::Atan2(DB.Dot(V), DB.Dot(U));
					});

					const int32 First = AddVertex(Polygon[0]);
					for (int32 Point = 1; Point + 1 < Polygon.Num(); ++Point)
					{
						const int32 B = AddVertex(Polygon[Point]);
						const int32 C = AddVertex(Polygon[Point + 1]);
						if (First != B && B != C && C != First)
						{
							OutMesh.AppendTriangle(First, C, B);
						}
					}
				}
			}
		}
	}
}
