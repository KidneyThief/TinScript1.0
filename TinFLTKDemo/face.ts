// ====================================================================================================================
// face.ts
// ====================================================================================================================

// -- execute the TinScriptDemo.ts, to define functions used to create a face
Exec("TinScriptDemo.ts");

// ====================================================================================================================
// Face demo
// ====================================================================================================================

void FacePiece::OnCreate()
{
    LinkNamespaces("FacePiece", "SceneObject");
    SceneObject::OnCreate();
    
    // -- a couple of attributes for face pieces
    bool self.alive = true;
    bool self.wink = false;
}

void Eye::OnCreate()
{
    LinkNamespaces("Eye", "FacePiece");
    FacePiece::OnCreate();
}

object Eye::OnUpdate()
{
    // -- cancel draw requests for this object
    CancelDrawRequests(self);
    
    if (self.wink)
    {
        DrawLine(self, self.position - '8 0 0', self.position + '8 0 0', 0);
    }
    else if (self.alive)
        DrawCircle(self, self.position, self.radius, 0);
    else
    {
        DrawLine(self, self.position - '8 8 0', self.position + '8 8 0', 0);
        DrawLine(self, self.position + '8 -8 0', self.position + '-8 8 0', 0);
    }
}

void Mouth::OnCreate()
{
    LinkNamespaces("Mouth", "FacePiece");
    FacePiece::OnCreate();
}

object Mouth::OnUpdate()
{
    // -- cancel draw requests for this object
    CancelDrawRequests(self);
    
    if (self.alive)
        DrawCircle(self, self.position, self.radius, 0);
    else
        DrawLine(self, self.position - '12 0 0', self.position + '12 0 0', 0);
}

void RightEye::OnCreate()
{
    LinkNamespaces("RightEye", "Eye");
    Eye::OnCreate();
}

void RightEye::ResetWink()
{
    self.wink = false;
}

// ====================================================================================================================
// Face implementation
// ====================================================================================================================
void CreateFace()
{
    CreateGame();
    CreateSceneObject("FacePiece", '320 240 0', 80);
    CreateSceneObject("RightEye", '300 200 0', 10);
    CreateSceneObject("Eye", '340 200 0', 10);
    CreateSceneObject("Mouth", '320 280 0', 30);
}

void KillFace(bool dead)
{
    object obj = gCurrentGame.game_objects.First();
    while (IsObject(obj))
    {
        obj.alive = !dead;
        obj = gCurrentGame.game_objects.Next();
    }
}

void Wink()
{
    object right_eye = FindObject("RightEye");
    if (IsObject(right_eye))
    {
        right_eye.wink = true;
        schedule(right_eye, 1000, Hash("ResetWink"));
    }
}

// ====================================================================================================================
// EOF
// ====================================================================================================================
