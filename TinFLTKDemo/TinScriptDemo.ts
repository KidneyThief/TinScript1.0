// ====================================================================================================================
// TinScriptDemo.ts
// ====================================================================================================================
Print("Welcome to the TinScript Demo!");

void ThreadTestCount(int num) {
    Print(num);
    if(num > 0)
        schedule(0, 1000, Hash("ThreadTestCount"), num - 1);
}

// ====================================================================================================================
// DefaultGame implementation
// ====================================================================================================================
void DefaultGame::OnCreate()
{
    int self.sched_update = schedule(self, 100, Hash("OnUpdate"));
    object self.game_objects = create CObjectGroup("GameObjects");
    int self.SimTime = GetSimTime();
}

void DefaultGame::OnDestroy()
{
    destroy self.game_objects;
}

void DefaultGame::OnUpdate()
{
    // -- find out how much sim time has elapsed
    int curTime = GetSimTime();
    float deltaTime = (curTime - self.SimTime);
    deltaTime /= 1000.0f;
    self.SimTime = curTime;
    
    object cur_object = self.game_objects.First();
    while (IsObject(cur_object))
    {
        cur_object.OnUpdate(deltaTime);
        cur_object = self.game_objects.Next();
    }
    
    // -- continue the thread
    self.sched_update = schedule(self, 1, Hash("OnUpdate"));
}

object gCurrentGame;
void CreateGame()
{
    if (!IsObject(gCurrentGame))
    {
        gCurrentGame = create CScriptObject("DefaultGame");
    }
}

void ResetGame()
{
    if (IsObject(gCurrentGame))
    {
        destroy gCurrentGame;
    }
    
    //  -- always start Unpaused
    SimUnpause();
}

// -- wrapper to handle events
void NotifyEvent(int keypress)
{
    if (IsObject(gCurrentGame))
    {
        gCurrentGame.OnKeyPress(keypress);
    }
}

// ====================================================================================================================
// SceneObject implementation
// ====================================================================================================================
void SceneObject::OnCreate()
{
    vector3f self.position;
    float self.radius;
}

void SceneObject::OnDestroy()
{
    // -- ensure we have no latent draw requests
    CancelDrawRequests(self);
}

void SceneObject::OnUpdate(float deltaTime)
{
    // -- we're using our object ID also as a draw request ID
    CancelDrawRequests(self);
    DrawCircle(self, self.position, self.radius, 0);
}

object CreateSceneObject(string name, vector3f position, float radius)
{
    // -- create the asteroid
    object scene_object = create CScriptObject(name);
        
    scene_object.position = position;
    if (scene_object.radius == 0.0f)
        scene_object.radius = radius;
    
    gCurrentGame.game_objects.AddObject(scene_object);
    
    // -- return the object create
    return (scene_object);
}

// ====================================================================================================================
// EOF
// ====================================================================================================================
