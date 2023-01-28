#ifdef EXPANSIONMODSPAWNSELECTION
modded class ExpansionRespawnHandlerModule
{
    bool IsPlayerInSpawnSelect(PlayerIdentity identity)
    {
        string uid = identity.GetId();
        ExpansionPlayerState state = m_PlayerStartStates.Get(uid);

        if ( state )
            return true;

        return false;
    }
};
#endif