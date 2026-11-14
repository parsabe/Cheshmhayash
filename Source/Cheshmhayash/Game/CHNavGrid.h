// Cheshm Hayash - a small 2D walkability grid with A* pathfinding.
// Levels are generated in code at runtime, so instead of a baked navmesh each stealth location
// rasterises its own blocking geometry into this grid once, right after it is built.

#pragma once

#include "CoreMinimal.h"

class UWorld;

struct FCHNavGrid
{
	/** Samples the world's static blocking geometry inside Bounds. FloorZ is the walking surface height. */
	void Build(UWorld* World, const FBox2D& Bounds, float InCellSize, float InFloorZ);

	bool IsValid() const { return Width > 0 && Height > 0; }

	/** Path from From to To as world points (excluding From, ending at To or the nearest reachable cell). */
	bool FindPath(const FVector& From, const FVector& To, TArray<FVector>& OutPoints) const;

	/** True if a straight walk between the two points crosses no blocked cell. */
	bool LineWalkable(const FVector& From, const FVector& To) const;

	bool IsWalkableAt(const FVector& Point) const;

	/** Snaps a point to the centre of the nearest walkable cell. */
	bool NearestWalkable(const FVector& Point, FVector& OutPoint, int32 MaxRing = 12) const;

private:
	bool InBounds(int32 X, int32 Y) const { return X >= 0 && Y >= 0 && X < Width && Y < Height; }
	bool IsOpen(int32 X, int32 Y) const { return InBounds(X, Y) && !Blocked[Y * Width + X]; }
	void ToCell(const FVector& Point, int32& X, int32& Y) const;
	FVector ToWorld(int32 X, int32 Y) const;
	bool NearestOpenCell(int32& X, int32& Y, int32 MaxRing) const;
	bool CellLineOpen(int32 X0, int32 Y0, int32 X1, int32 Y1) const;

	FVector2D Origin = FVector2D::ZeroVector;
	float CellSize = 100.f;
	float FloorZ = 0.f;
	int32 Width = 0;
	int32 Height = 0;
	TArray<uint8> Blocked;
};
