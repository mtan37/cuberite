
#pragma once

#include "BlockEntityWithItems.h"
#include "../Simulator/RedstoneSimulator.h"





class cClientHandle;





// tolua_begin
class cChestEntity :
	public cBlockEntityWithItems
{
	// tolua_end

	using Super = cBlockEntityWithItems;

	// tolua_begin

public:

	enum
	{
		ContentsHeight = 3,
		ContentsWidth  = 9,
	} ;

	// tolua_end

	BLOCKENTITY_PROTODEF(cChestEntity)

	/** Constructor used for normal operation */
	cChestEntity(BLOCKTYPE a_BlockType, NIBBLETYPE a_BlockMeta, Vector3i a_Pos, cWorld * a_World);

	virtual ~cChestEntity() override;

	// cBlockEntity overrides:
	virtual void CopyFrom(const cBlockEntity & a_Src) override;
	virtual void SendTo(cClientHandle & a_Client) override;
	virtual bool UsedBy(cPlayer * a_Player) override;

	/** Search horizontally adjacent blocks for neighbouring chests of the same type and links them together. */
	void ScanNeighbours();

	/** Opens a new chest window where this is the primary chest and any neighbour is the secondary. */
	void OpenNewWindow();

	/** Forces any players to close the owned window. */
	void DestroyWindow();

	/** Returns true if the chest should not be accessible by players. */
	bool IsBlocked();

	/** Gets the number of players who currently have this chest open */
	int GetNumberOfPlayers(void) const { return m_NumActivePlayers; }

	/** Sets the number of players who currently have this chest open */
	void SetNumberOfPlayers(int a_NumActivePlayers) { m_NumActivePlayers = a_NumActivePlayers; }


private:

	/** Number of players who currently have this chest open */
	int m_NumActivePlayers;

	/** Neighbouring chest that links to form a double chest */
	cChestEntity * m_Neighbour;

	/** cItemGrid::cListener overrides: */
	virtual void OnSlotChanged(cItemGrid * a_Grid, int a_SlotNum) override
	{
		ASSERT(a_Grid == &m_Contents);

		if (m_World == nullptr)
		{
			return;
		}

		// Have cBlockEntityWithItems update redstone and try to broadcast our window:
		Super::OnSlotChanged(a_Grid, a_SlotNum);

		cWindow * Window = GetWindow();
		if ((Window == nullptr) && (m_Neighbour != nullptr))
		{
			// Window was null, Super will have failed.
			// Neighbour might own the window:
			Window = m_Neighbour->GetWindow();
		}

		if (Window != nullptr)
		{
			Window->BroadcastWholeWindow();
		}

		m_World->MarkChunkDirty(GetChunkX(), GetChunkZ());
		m_World->DoWithChunkAt(m_Pos, [&](cChunk & a_Chunk)
		{
			auto & Simulator = *m_World->GetRedstoneSimulator();

			// Notify comparators:
			Simulator.WakeUp(m_Pos + Vector3i(1, 0, 0), &a_Chunk);
			Simulator.WakeUp(m_Pos + Vector3i(-1, 0, 0), &a_Chunk);
			Simulator.WakeUp(m_Pos + Vector3i(0, 0, 1), &a_Chunk);
			Simulator.WakeUp(m_Pos + Vector3i(0, 0, -1), &a_Chunk);
			return true;
		});
	}

} ;  // tolua_export




