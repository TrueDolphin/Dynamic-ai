



/*
  idea. changed idea.
      //TrailTrigger(ai, player);

  //trigger and looped delete/create so they follow the player.
  void TrailTrigger(eAIBase ai, PlayerBase player) {
    Reactive_Trigger Reactive_trigger = Reactive_Trigger.Cast(GetGame().CreateObjectEx("Reactive_Trigger", ai.GetPosition(), ECE_NONE));
    Reactive_trigger.SetCollisionCylinder(90, 90 / 2);
    Reactive_trigger.Dynamic_SetData(ai, player, 60, 5000);
    thread TrailTimer(ai, player, Reactive_trigger);
  }
  void TrailTimer(eAIBase ai, PlayerBase player, Reactive_Trigger trigger) {
    Sleep(10000);
    if (!ai || !player || !trigger) return;
    trigger.Delete();
    Reactive_Trigger Reactive_trigger = Reactive_Trigger.Cast(GetGame().CreateObjectEx("Reactive_Trigger", ai.GetPosition(), ECE_NONE));
    Reactive_trigger.SetCollisionCylinder(90, 90 / 2);
    Reactive_trigger.Dynamic_SetData(ai, player, 60, 5000);
    thread TrailTimer(ai, player, Reactive_trigger);
  }
 */


class Reactive_Trigger: CylinderTrigger {
  eAIBase c_ai;
  PlayerBase c_player;
  int c_TailDistance = 80;
  int c_Cooldown = 5000;
  bool c_allow = true;
  bool c_running = false;

  void Dynamic_SetData(eAIBase a, PlayerBase b, int c, int d) {
    c_ai = a;
    c_player = b;
    c_TailDistance = c;
    c_Cooldown = d;
  }

  override void Enter(TriggerInsider insider) {
    super.Enter(insider);
    PlayerBase player = PlayerBase.Cast(insider.GetObject());
  }

  override void Leave(TriggerInsider insider) {
    super.Leave(insider);
    PlayerBase player = PlayerBase.Cast(insider.GetObject());

    if (!c_ai || !c_player) {
      if (this) Delete();
      return;
    }
    if (player == c_player) {
      if (!c_running) {
        thread AllowTail(this);
        eAIGroup AiGroup = eAIGroup.Cast(c_ai.GetGroup());
        if (!AiGroup) AiGroup = eAIGroup.GetGroupByLeader(c_ai);
        c_ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), c_TailDistance, c_TailDistance + 20));
      }
    }
  }

  void AllowTail(Reactive_Trigger trigger) {
    c_running = true;
    Sleep(c_Cooldown);
    if (!trigger) return;
    c_running = false;
  }

  override protected bool CanAddObjectAsInsider(Object object) {

    if (!super.CanAddObjectAsInsider(object)) {
      return false;
    }
    return PlayerBase.Cast(object) != null;
  }
}