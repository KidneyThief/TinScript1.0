// ====================================================================================================================
// missions.ts
// ====================================================================================================================

// -- this game is built upon the TinScriptDemo
Exec("TinScriptDemo.ts");

// -- global tunables -------------------------------------------------------------------------------------------------
float gThrust = 12.0f;
float gAsteroidSpeed = 52.0f;
float gBulletSpeed = 150.0f;

int gMaxBullets = 4;
int gFireCDTime = 100;

// -- FLTK Colors -----------------------------------------------------------------------------------------------------
int gFLTK_BLACK = 0;
int gFLTK_RED = 1;
int gFLTK_GREEN = 2;
int gFLTK_YELLOW = 3;
int gFLTK_BLUE = 4;
int gFLTK_PURPLE = 5;
int gFLTK_CYAN = 6;
int gFLTK_BROWN = 9;

// == TriggerVolume ===================================================================================================

void TriggerVolume::OnCreate()
{
    LinkNamespaces("Asteroid", "SceneObject");
    SceneObject::OnCreate();
    
    bool self.enabled = true;
    int self.color = gFLTK_BLUE;
}

void TriggerVolume::OnUpdate(float deltaTime)
{
    // -- draw the trigger volume
    CancelDrawRequests(self);
    
    if (!self.enabled)
        return;
    
    float offset = self.radius;
    vector3f top_left = self.position;
    top_left:x -= offset;
    top_left:y -= offset;
    
    vector3f top_right = self.position;
    top_left:x += offset;
    top_left:y -= offset;

    vector3f bottom_left = self.position;
    top_left:x -= offset;
    top_left:y += offset;
    
    vector3f bottom_right = self.position;
    top_left:x += offset;
    top_left:y += offset;
    
    DrawLine(self, top_left, top_right, self.color);
    DrawLine(self, top_right, bottom_right, self.color);
    DrawLine(self, bottom_right, bottom_left, self.color);
    DrawLine(self, bottom_left, top_left, self.color);
}

