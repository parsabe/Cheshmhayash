// Cheshm Hayash - walkability grid with A* pathfinding.

#include "Game/CHNavGrid.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/World.h"

void FCHNavGrid::Build(UWorld* World, const FBox2D& Bounds, float InCellSize, float InFloorZ)
{
	CellSize = FMath::Max(25.f, InCellSize);
	FloorZ = InFloorZ;
	Origin = Bounds.Min;
	Width = FMath::Max(1, FMath::CeilToInt32((Bounds.Max.X - Bounds.Min.X) / CellSize));
	Height = FMath::Max(1, FMath::CeilToInt32((Bounds.Max.Y - Bounds.Min.Y) / CellSize));
	Blocked.SetNumZeroed(Width * Height);

	if (!World)
	{
		return;
	}

	// a person-sized box hovering above the ground: anything static it touches blocks the cell
	const FCollisionShape Shape = FCollisionShape::MakeBox(FVector(CellSize * 0.5f + 8.f, CellSize * 0.5f + 8.f, 55.f));
	const FCollisionObjectQueryParams ObjectParams(ECC_WorldStatic);
	FCollisionQueryParams QueryParams(FName(TEXT("CHNavGrid")), false);

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const FVector Center = ToWorld(X, Y) + FVector(0.f, 0.f, 100.f);
			Blocked[Y * Width + X] = World->OverlapAnyTestByObjectType(Center, FQuat::Identity, ObjectParams, Shape, QueryParams) ? 1 : 0;
		}
	}
}

void FCHNavGrid::ToCell(const FVector& Point, int32& X, int32& Y) const
{
	X = FMath::FloorToInt32((Point.X - Origin.X) / CellSize);
	Y = FMath::FloorToInt32((Point.Y - Origin.Y) / CellSize);
}

FVector FCHNavGrid::ToWorld(int32 X, int32 Y) const
{
	return FVector(Origin.X + (X + 0.5f) * CellSize, Origin.Y + (Y + 0.5f) * CellSize, FloorZ);
}

bool FCHNavGrid::IsWalkableAt(const FVector& Point) const
{
	int32 X = 0;
	int32 Y = 0;
	ToCell(Point, X, Y);
	return IsOpen(X, Y);
}

bool FCHNavGrid::NearestOpenCell(int32& X, int32& Y, int32 MaxRing) const
{
	if (IsOpen(X, Y))
	{
		return true;
	}
	for (int32 Ring = 1; Ring <= MaxRing; ++Ring)
	{
		int32 BestX = 0;
		int32 BestY = 0;
		int32 BestDist = MAX_int32;
		for (int32 DY = -Ring; DY <= Ring; ++DY)
		{
			for (int32 DX = -Ring; DX <= Ring; ++DX)
			{
				if (FMath::Max(FMath::Abs(DX), FMath::Abs(DY)) != Ring)
				{
					continue;
				}
				if (IsOpen(X + DX, Y + DY))
				{
					const int32 Dist = DX * DX + DY * DY;
					if (Dist < BestDist)
					{
						BestDist = Dist;
						BestX = X + DX;
						BestY = Y + DY;
					}
				}
			}
		}
		if (BestDist != MAX_int32)
		{
			X = BestX;
			Y = BestY;
			return true;
		}
	}
	return false;
}

bool FCHNavGrid::NearestWalkable(const FVector& Point, FVector& OutPoint, int32 MaxRing) const
{
	int32 X = 0;
	int32 Y = 0;
	ToCell(Point, X, Y);
	if (!IsValid() || !NearestOpenCell(X, Y, MaxRing))
	{
		return false;
	}
	OutPoint = ToWorld(X, Y);
	return true;
}

bool FCHNavGrid::CellLineOpen(int32 X0, int32 Y0, int32 X1, int32 Y1) const
{
	// walk the segment in small steps; cheap and good enough at this grid size
	const int32 Steps = FMath::Max(FMath::Abs(X1 - X0), FMath::Abs(Y1 - Y0)) * 3 + 1;
	for (int32 Step = 0; Step <= Steps; ++Step)
	{
		const float T = static_cast<float>(Step) / Steps;
		const float FX = FMath::Lerp(X0 + 0.5f, X1 + 0.5f, T);
		const float FY = FMath::Lerp(Y0 + 0.5f, Y1 + 0.5f, T);
		if (!IsOpen(FMath::FloorToInt32(FX), FMath::FloorToInt32(FY)))
		{
			return false;
		}
	}
	return true;
}

bool FCHNavGrid::LineWalkable(const FVector& From, const FVector& To) const
{
	if (!IsValid())
	{
		return true;
	}
	int32 X0 = 0;
	int32 Y0 = 0;
	int32 X1 = 0;
	int32 Y1 = 0;
	ToCell(From, X0, Y0);
	ToCell(To, X1, Y1);
	return CellLineOpen(X0, Y0, X1, Y1);
}

bool FCHNavGrid::FindPath(const FVector& From, const FVector& To, TArray<FVector>& OutPoints) const
{
	OutPoints.Reset();
	if (!IsValid())
	{
		OutPoints.Add(To);
		return true;
	}

	int32 StartX = 0;
	int32 StartY = 0;
	int32 GoalX = 0;
	int32 GoalY = 0;
	ToCell(From, StartX, StartY);
	ToCell(To, GoalX, GoalY);
	StartX = FMath::Clamp(StartX, 0, Width - 1);
	StartY = FMath::Clamp(StartY, 0, Height - 1);
	GoalX = FMath::Clamp(GoalX, 0, Width - 1);
	GoalY = FMath::Clamp(GoalY, 0, Height - 1);
	if (!NearestOpenCell(StartX, StartY, 6) || !NearestOpenCell(GoalX, GoalY, 14))
	{
		return false;
	}

	const int32 NumCells = Width * Height;
	const int32 StartIndex = StartY * Width + StartX;
	const int32 GoalIndex = GoalY * Width + GoalX;

	TArray<float> Cost;
	Cost.Init(TNumericLimits<float>::Max(), NumCells);
	TArray<int32> Parent;
	Parent.Init(INDEX_NONE, NumCells);
	TArray<uint8> Closed;
	Closed.SetNumZeroed(NumCells);

	struct FOpenNode
	{
		float Priority;
		int32 Index;
		bool operator<(const FOpenNode& Other) const { return Priority < Other.Priority; }
	};
	TArray<FOpenNode> Open;
	Open.HeapPush(FOpenNode{ 0.f, StartIndex });
	Cost[StartIndex] = 0.f;

	auto Heuristic = [GoalX, GoalY](int32 X, int32 Y)
	{
		const int32 DX = FMath::Abs(X - GoalX);
		const int32 DY = FMath::Abs(Y - GoalY);
		return static_cast<float>(FMath::Max(DX, DY)) + 0.4142f * FMath::Min(DX, DY);
	};

	bool bFound = false;
	while (Open.Num() > 0)
	{
		FOpenNode Node;
		Open.HeapPop(Node, EAllowShrinking::No);
		if (Closed[Node.Index])
		{
			continue;
		}
		Closed[Node.Index] = 1;
		if (Node.Index == GoalIndex)
		{
			bFound = true;
			break;
		}

		const int32 X = Node.Index % Width;
		const int32 Y = Node.Index / Width;
		for (int32 DY = -1; DY <= 1; ++DY)
		{
			for (int32 DX = -1; DX <= 1; ++DX)
			{
				if (DX == 0 && DY == 0)
				{
					continue;
				}
				const int32 NX = X + DX;
				const int32 NY = Y + DY;
				if (!IsOpen(NX, NY))
				{
					continue;
				}
				// no cutting corners around buildings
				if (DX != 0 && DY != 0 && (!IsOpen(X + DX, Y) || !IsOpen(X, Y + DY)))
				{
					continue;
				}
				const int32 NeighbourIndex = NY * Width + NX;
				if (Closed[NeighbourIndex])
				{
					continue;
				}
				const float NewCost = Cost[Node.Index] + ((DX != 0 && DY != 0) ? 1.4142f : 1.f);
				if (NewCost < Cost[NeighbourIndex])
				{
					Cost[NeighbourIndex] = NewCost;
					Parent[NeighbourIndex] = Node.Index;
					Open.HeapPush(FOpenNode{ NewCost + Heuristic(NX, NY), NeighbourIndex });
				}
			}
		}
	}

	if (!bFound)
	{
		return false;
	}

	// walk back, then drop every point that a straight line can skip
	TArray<int32> Cells;
	for (int32 Index = GoalIndex; Index != INDEX_NONE; Index = Parent[Index])
	{
		Cells.Add(Index);
	}
	Algo::Reverse(Cells);

	int32 Anchor = 0;
	while (Anchor < Cells.Num() - 1)
	{
		int32 Next = Cells.Num() - 1;
		while (Next > Anchor + 1)
		{
			if (CellLineOpen(Cells[Anchor] % Width, Cells[Anchor] / Width, Cells[Next] % Width, Cells[Next] / Width))
			{
				break;
			}
			--Next;
		}
		OutPoints.Add(ToWorld(Cells[Next] % Width, Cells[Next] / Width));
		Anchor = Next;
	}

	if (OutPoints.Num() == 0)
	{
		OutPoints.Add(ToWorld(GoalX, GoalY));
	}
	// finish exactly on the requested point when it is walkable
	if (IsWalkableAt(To))
	{
		OutPoints.Last() = FVector(To.X, To.Y, FloorZ);
	}
	return true;
}
