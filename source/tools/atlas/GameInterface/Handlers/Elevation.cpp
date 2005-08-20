#include "precompiled.h"

#include "MessageHandler.h"

#include "../CommandProc.h"

#include "graphics/Terrain.h"
#include "ps/Game.h"

namespace AtlasMessage {

BEGIN_COMMAND(AlterElevation)

	// TODO: much more efficient version of this, and without the memory leaks
	u16* OldTerrain;
	u16* NewTerrain;

	void Construct()
	{
		OldTerrain = NewTerrain = NULL;
	}
	void Destruct()
	{
		delete OldTerrain;
		delete NewTerrain;
	}

	void Do() {

		CTerrain* terrain = g_Game->GetWorld()->GetTerrain();

		int verts = terrain->GetVerticesPerSide()*terrain->GetVerticesPerSide();
		OldTerrain = new u16[verts];
		memcpy(OldTerrain, terrain->GetHeightMap(), verts*sizeof(u16));

		int x = (int)d->pos.x;
		int z = (int)d->pos.z;
		terrain->RaiseVertex(x, z, (int)d->amount);
		terrain->MakeDirty(x, z, x, z);

	}

	void Undo() {
		CTerrain* terrain = g_Game->GetWorld()->GetTerrain();
		if (! NewTerrain)
		{
			int verts = terrain->GetVerticesPerSide()*terrain->GetVerticesPerSide();
			NewTerrain = new u16[verts];
			memcpy(NewTerrain, terrain->GetHeightMap(), verts*sizeof(u16));
		}
		terrain->SetHeightMap(OldTerrain); // CTerrain duplicates the data
	}

	void Redo() {
		CTerrain* terrain = g_Game->GetWorld()->GetTerrain();
		terrain->SetHeightMap(NewTerrain); // CTerrain duplicates the data
	}

	void MergeWithSelf(cAlterElevation* prev) {
		std::swap(prev->NewTerrain, NewTerrain);
	}

END_COMMAND(AlterElevation);

}
