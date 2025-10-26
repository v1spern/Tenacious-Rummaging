// Action registration

modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);

        TR_Debug.Log("[ActionConstructor] Registering Tenacious Rummaging actions");

        actions.Insert(ActionSearchNode);

        actions.Insert(ActionAddStaticSearchNode);
        actions.Insert(ActionAddClassSearchNode);

        actions.Insert(ActionAddInteriorPieceSearchNode);


        TR_Debug.Log("[ActionConstructor] All Tenacious Rummaging actions registered");
    }
}
