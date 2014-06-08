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

int gTeam_Neutral = 0;
int gTeam_Ally = 1;
int gTeam_Enemy = 2;
float gPlayer_Radius = 20.0f;
float gEnemy_Radius = 15.0f;

// -- FLTK Colors -----------------------------------------------------------------------------------------------------
int gFLTK_BLACK = 0;
int gFLTK_RED = 1;
int gFLTK_GREEN = 2;
int gFLTK_YELLOW = 3;
int gFLTK_BLUE = 4;
int gFLTK_PURPLE = 5;
int gFLTK_CYAN = 6;
int gFLTK_BROWN = 9;

// ====================================================================================================================
// Helper functions
// ====================================================================================================================
void UpdateScreenPosition(object obj, float deltaTime)
{
    if (IsObject(obj))
    {
        // -- apply the velocity
        obj.position = obj.position + (obj.velocity * deltaTime);

        // -- wrap the object around the screen edge
        if (obj.position:x < 0.0f)
        {
            obj.position:x = 0.0f;
            obj.velocity:x = 0.0f;
        }
        else if (obj.position:x > 640.0f)
        {
            obj.position:x = 640.0f;
            obj.velocity:x = 0.0f;
        }
            
        if (obj.position:y < 0.0f)
        {
            obj.position:y = 0.0f;
            obj.velocity:y = 0.0f;
        }
        else if (obj.position:y > 480)
        {
            obj.position:y = 480.0f;
            obj.velocity:y = 0.0f;
        }
    }
}

// ====================================================================================================================
// TriggerVolume implementation
// ====================================================================================================================
void TriggerVolume::OnCreate()
{
    LinkNamespaces("TriggerVolume", "SceneObject");
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

// ====================================================================================================================
// Character implementation
// ====================================================================================================================
void Character::OnCreate()
{
    LinkNamespaces("Character", "SceneObject");
    SceneObject::OnCreate();
    
    // -- characters have velocity and rotation
    vector3f self.velocity = '0 0 0';
    float self.rotation = 270.0f;
    
    // -- life count
    int self.health = 100;
    int self.show_hit_time = 0;
    
    // -- team
    int self.team = gTeam_Neutral;
    
    // -- we can only fire so fast
    int self.fire_cd_time = 0;
    
    // -- add ourself to the game's character set
    if (IsObject(gCurrentGame))
    {
        gCurrentGame.character_set.AddObject(self);
    }
}

void Character::OnUpdate(float deltaTime)
{
    // -- update the screen position - applies the velocity, and wraps
    UpdateScreenPosition(self, deltaTime);
    
    // -- we look like a triangle
    float head_offset_x = self.radius * Cos(self.rotation);
    float head_offset_y = self.radius * Sin(self.rotation);
    vector3f head = self.position;
    head:x = head:x + head_offset_x;
    head:y = head:y + head_offset_y;
    
    float tail0_offset_x = self.radius * Cos(self.rotation + 120.0f);
    float tail0_offset_y = self.radius * Sin(self.rotation + 120.0f);
    vector3f tail0 = self.position;
    tail0:x += tail0_offset_x;
    tail0:y += tail0_offset_y;

    float tail1_offset_x = self.radius * Cos(self.rotation + 240.0f);
    float tail1_offset_y = self.radius * Sin(self.rotation + 240.0f);
    vector3f tail1 = self.position;
    tail1:x += tail1_offset_x;
    tail1:y += tail1_offset_y;
    
    // -- cancel all draws for this
    CancelDrawRequests(self);
    
    // -- determine our color
    int color = gFLTK_BLACK;
    if (self.team == gTeam_Ally)
        color = gFLTK_BLUE;
    else if (self.team == gTeam_Enemy)
        color = gFLTK_PURPLE;
    int cur_time = GetSimTime();
    if (cur_time < self.show_hit_time || self.health <= 0)
        color = gFLTK_RED;
    
    // -- draw the 4 lines creating the "triangle-ish" ship
    DrawLine(self, head, tail0, color);
    DrawLine(self, head, tail1, color);
    DrawLine(self, tail0, self.position, color);
    DrawLine(self, tail1, self.position, color);
    
    // -- if we're dead, draw the text
    if (self.health <= 0)
        DrawText(self, self.position - '15 0 0', "DEAD", gFLTK_RED);
}

void Character::ApplyThrust(float thrust)
{
    // -- calculate our heading
    vector3f heading = '0 0 0';
    heading:x = Cos(self.rotation);
    heading:y = Sin(self.rotation);
    
    // -- Apply the an impulse in the direction we're heading
    ApplyImpulse(self, heading * thrust);
}

void Character::OnFire()
{
    // -- no shooting when we're dead
    if (self.health <= 0)
        return;
        
    // -- only 4x bullets at a time
    if (gCurrentGame.bullet_set.Used() >= gMaxBullets)
        return;
    
    // -- only allow one bullet every X msec
    int cur_time = GetSimTime();
    if (cur_time < self.fire_cd_time)
        return;
        
    // -- calculate our heading
    vector3f heading = '0 0 0';
    heading:x = Cos(self.rotation);
    heading:y = Sin(self.rotation);
    
    // -- find the nose of the ship
    vector3f head = self.position + (heading * self.radius);
    
    // -- spawn a bullet
    SpawnBullet(self, head, heading);
    
    // -- start a cooldown
    self.fire_cd_time = cur_time + gFireCDTime;
}

void Character::OnDeath()
{
    destroy self;
}

void Character::OnCollision()
{
    if (self.health <= 0)
        return;
    
    // -- decrement the health
    self.health -= 1;
    
    // -- if we're dead, schedule the object to be deleted
    if (self.health <= 0)
    {
        schedule(self, 2000, Hash("OnDeath"));
        return;
    }
    self.foo = "cat";
}

// ====================================================================================================================
// Bullet implementation
// ====================================================================================================================
void Bullet::OnCreate()
{
    LinkNamespaces("Bullet", "SceneObject");
    SceneObject::OnCreate();
    
    // -- Bullets are shot by characters
    object self.source;
    
    // -- Bullets have velocity
    vector3f self.velocity = '0 0 0';
    
    // -- self terminating
    int cur_time = GetSimTime();
    int self.expireTime = cur_time + 2000;
}

void Bullet::OnUpdate(float deltaTime)
{
    // -- update the screen position - applies the velocity, and wraps
    UpdateScreenPosition(self, deltaTime);
    
    // -- we're using our object ID also as a draw request ID
    CancelDrawRequests(self);
    DrawCircle(self, self.position, self.radius, gFLTK_RED);
    
    // -- see if it's time to expire
    int cur_time = GetSimTime();
    if (cur_time > self.expireTime)
        destroy self;
}

object SpawnBullet(object source, vector3f position, vector3f direction)
{
    object bullet = CreateSceneObject("Bullet", position, 3);
    bullet.source = source;
    
    // -- apply the muzzle velocity
    ApplyImpulse(bullet, direction * gBulletSpeed);
    
    // -- add the bullet to the game's bullet set
    if (IsObject(gCurrentGame))
        gCurrentGame.bullet_set.AddObject(bullet);
}

// ====================================================================================================================
// MissionSim implementation
// ====================================================================================================================
void MissionSim::OnCreate()
{
    LinkNamespaces("MissionSim", "DefaultGame");
    DefaultGame::OnCreate();
    
    // -- cache the 'player' object
    object self.player;
    
    // -- create sets for iterating/finding objects
    object self.character_set = create CObjectSet("CharacterSet");
    object self.bullet_set = create CObjectSet("BulletSet");
    object self.delete_set = create CObjectSet("DeleteSet");
}

void MissionSim::OnDestroy()
{
    // -- call the default
    DefaultGame::OnDestroy();
    
    // -- clean up the extra sets
    destroy self.character_set;
    destroy self.bullet_set;
    destroy self.delete_set;
}

void MissionSim::OnUpdate()
{
    // -- update all the scene objects
    DefaultGame::OnUpdate();
    
    object bullet = self.bullet_set.First();
    while (IsObject(bullet))
    {
        // -- loop through all the asteroids
        bool finished = false;
        object character = self.character_set.First();
        while (!finished && IsObject(character))
        {
            if (character.team != gTeam_Neutral && bullet.source.team != character.team)
            {
                // -- if the bullet is within radius of the character, it's a collision
                float distance = V3fLength(bullet.position - character.position);
                if (distance < character.radius)
                {
                    // -- set the flag, call the function
                    character.OnCollision();
                    
                    // -- add the bullet to the delete set and break
                    self.delete_set.AddObject(bullet);
                    
                    // $$$TZA break doesn't compile??
                    finished = true;
                }
            }
            
            // -- get the next character
            character = self.character_set.Next();
        }
        
        // -- check the next bullet
        bullet = self.bullet_set.Next();
    }
    
    // -- now clean up the delete set
    while (self.delete_set.Used() > 0)
    {
        object delete_me = self.delete_set.GetObjectByIndex(0);
        destroy delete_me;
    }
}

void MissionSim::OnKeyPress(int keypress)
{
    // -- rotate left
    if (keypress == CharToInt('j'))
    {
        if (IsObject(gCurrentGame.player))
            gCurrentGame.player.rotation -= 10.0f;
    }
    
    // -- rotate right
    else if (keypress == CharToInt('l'))
    {
        if (IsObject(gCurrentGame.player))
            gCurrentGame.player.rotation += 10.0f;
    }
    
    // -- thrust
    else if (keypress == CharToInt('i'))
    {
        if (IsObject(gCurrentGame.player))
            gCurrentGame.player.ApplyThrust(gThrust);
    }
    
    // -- fire
    else if (keypress == CharToInt(' '))
    {
        if (IsObject(gCurrentGame.player))
            gCurrentGame.player.OnFire();
    }
}

void ApplyImpulse(object obj, vector3f impulse)
{
    if (IsObject(obj))
    {
        obj.velocity = obj.velocity + impulse;
    }
}

// ====================================================================================================================
// Player
// ====================================================================================================================
void Player::OnCreate()
{
    LinkNamespaces("Player", "Character");
    Character::OnCreate();
    
    // -- set the team
    self.team = gTeam_Ally;
    
    // -- let the game know who to send key events to
    gCurrentGame.player = self;
    
    // -- override the radius
    self.radius = gPlayer_Radius;
}

// ====================================================================================================================
// Enemy
// ====================================================================================================================
void Enemy::OnCreate()
{
    LinkNamespaces("Enemy", "Character");
    Character::OnCreate();
    
    // -- set the team
    self.team = gTeam_Enemy;
    
    // -- override the radius
    self.radius = gEnemy_Radius;
}

// ====================================================================================================================
// Mission functions
// ====================================================================================================================
object SpawnCharacter(string char_name, vector3f position)
{
    object new_char = CreateSceneObject(char_name, position, 20);
    return (new_char);
}

void StartMission()
{
    ResetGame();
    gCurrentGame = create CScriptObject("MissionSim");
    
    SpawnCharacter("Player", "320 240 0");
    SpawnCharacter("Enemy", "320 100 0");
}

// ====================================================================================================================
// EOF
// ====================================================================================================================


