// ====================================================================================================================
// asteroids.ts
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


// ====================================================================================================================
// Asteroid : SceneObject implementation
// ====================================================================================================================
void Asteroid::OnCreate()
{
    LinkNamespaces("Asteroid", "SceneObject");
    SceneObject::OnCreate();
    
    // -- asteroids have movement
    vector3f self.velocity = '0 0 0';
    
    // -- add ourself to the game's asteroid set
    if (IsObject(gCurrentGame))
    {
        gCurrentGame.asteroid_set.AddObject(self);
    }
}

void Asteroid::OnUpdate(float deltaTime)
{
    // -- update the screen position - applies the velocity, and wraps
    UpdateScreenPosition(self, deltaTime);
        
    // -- draw the asteroid
    CancelDrawRequests(self);
    DrawCircle(self, self.position, self.radius, 0);
}

void Asteroid::OnCollision()
{
    // -- get the asteroid heading
    vector3f velocity = self.velocity;
    float speed = V3fLength(velocity);
    float heading = Atan2(velocity:y, velocity:x);
    vector3f new_heading0 = '0 0 0';
    new_heading0:x = Cos(heading + 20.0f + (Random() * 40.0f));
    new_heading0:y = Sin(heading + 20.0f + (Random() * 40.0f));
    
    vector3f new_heading1 = '0 0 0';
    new_heading1:x = Cos(heading - 20.0f - (Random() * 40.0f));
    new_heading1:y = Sin(heading - 20.0f - (Random() * 40.0f));
    
    // -- if it's a large asteroid (radius == 50), split into two 30s
    bool should_split = (self.radius == 50 || self.radius == 30);
    float new_radius = 30;
    float new_speed_scale = 1.5f;
    if (self.radius == 30)
    {
        new_radius = 15;
        new_speed_scale = 2.5f;
    }
    
    if (should_split)
    {
        // -- two new asteroids, each spawned from the original position
        // -- each heading in a split direction, up to 90deg, but at twice the speed
        object asteroid0 = CreateSceneObject("Asteroid", self.position, new_radius);
        ApplyImpulse(asteroid0, new_heading0 * (speed * new_speed_scale));
        
        object asteroid1 = CreateSceneObject("Asteroid", self.position, new_radius);
        ApplyImpulse(asteroid1, new_heading1 * (speed * new_speed_scale));
        
        // -- and delete the original asteroid
        destroy self;
    }
    
    // -- otherwise, we simply destroy the object hit
    else
    {
        destroy self;
    }
}

// ====================================================================================================================
// SpawnAsteroid():  Create an 'Asteroid' SceneObject at a random location, not in the center
// ====================================================================================================================
object SpawnAsteroid()
{
    // -- calculate a random spawn direction
    vector3f offset_dir = '0 0 0';
    offset_dir:x = -1.0f + 2.0f * Random();
    offset_dir:y = -1.0f + 2.0f * Random();
    offset_dir = V3fNormalized(offset_dir);
    
    // -- now we need a random distance, > 140.0f so it's not in the center of the arena
    float offset_dist = 140.0f + 100.0f * Random();
    
    // -- the actual spawn position is from the center, in the offset_dir, at the random length
    vector3f spawn_pos = '320 240 0' + (offset_dir * offset_dist);
    
    object new_asteroid = CreateSceneObject("Asteroid", spawn_pos, 50);
    
    // -- random impulse
    vector3f impulse = '0 0 0';
    impulse:x = -1.0f + 2.0f * Random();
    impulse:y = -1.0f + 2.0f * Random();
    impulse = V3fNormalized(impulse);
    
    // -- apply a random strength
    float strength = (gAsteroidSpeed * 0.2f) + ((gAsteroidSpeed * 0.8f) * Random());
    impulse *= strength;
    
    // -- random 
    ApplyImpulse(new_asteroid, impulse);
}

void ApplyImpulse(object obj, vector3f impulse)
{
    if (IsObject(obj))
    {
        obj.velocity = obj.velocity + impulse;
    }
}

void UpdateScreenPosition(object obj, float deltaTime)
{
    if (IsObject(obj))
    {
        // -- apply the velocity
        obj.position = obj.position + (obj.velocity * deltaTime);

        // -- wrap the object around the screen edge
        if (obj.position:x < 0.0f)
            obj.position:x = 640.0f;
        else if (obj.position:x > 640.0f)
            obj.position:x = 0.0f;
            
        if (obj.position:y < 0.0f)
            obj.position:y = 480.0f;
        else if (obj.position:y > 480)
            obj.position:y = 0.0f;
    }
}

// ====================================================================================================================
// Ship implementation
// ====================================================================================================================

void Ship::OnCreate()
{
    LinkNamespaces("Ship", "SceneObject");
    SceneObject::OnCreate();
    
    // -- ships have velocity and rotation
    vector3f self.velocity = '0 0 0';
    float self.rotation = 270.0f;
    
    // -- life count
    int self.lives = 5;
    int self.show_hit_sched = 0;
    bool self.show_hit = false;
    vector3f self.show_hit_position;
    
    // -- on spawn, we're invulnerable for a couple seconds
    bool self.invulnerable = true;
    schedule(self, 2000, Hash("ResetInvulnerable"));
    
    // -- we can only fire so fast
    int self.fire_cd_time = 0;
}

void Ship::OnUpdate(float deltaTime)
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
    
    // -- draw the 4 lines creating the "triangle-ish" ship
    DrawLine(self, head, tail0, gFLTK_BLUE);
    DrawLine(self, head, tail1, gFLTK_BLUE);
    DrawLine(self, tail0, self.position, gFLTK_BLUE);
    DrawLine(self, tail1, self.position, gFLTK_BLUE);
    
    // -- draw if we're hit
    if (self.show_hit)
    {
        DrawText(self, self.show_hit_position, "OUCH!", gFLTK_RED);
    }
}

void Ship::ApplyThrust(float thrust)
{
    // -- calculate our heading
    vector3f heading = '0 0 0';
    heading:x = Cos(self.rotation);
    heading:y = Sin(self.rotation);
    
    // -- Apply the an impulse in the direction we're heading
    ApplyImpulse(self, heading * thrust);
}

void Ship::OnFire()
{
    // -- no shooting when we're dead
    if (self.lives <= 0)
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
    SpawnBullet(head, heading);
    
    // -- start a cooldown
    self.fire_cd_time = cur_time + gFireCDTime;
}

void Ship::ResetShowHit()
{
    self.show_hit = false;
}

void Ship::ResetInvulnerable()
{
    self.invulnerable = false;
}

void Ship::OnCollision()
{
    // -- do nothing if we're invulnerable
    if (self.invulnerable)
        return;
        
    // -- decrement the lives
    self.lives -= 1;
    
    // -- if we're dead, simply restart the entire game
    
    if (self.lives <= 0)
    {
        // -- game over, at the center of the screen
        DrawText(self, '280 230 0', "G A M E   O V E R", gFLTK_RED);
        
        SimPause();
        Print("Type StartAsteroids(); to continue...");
        return;
    }
    
    // -- if we're dead, game over
    self.show_hit = true;
    self.show_hit_position = self.position;
    
    // -- set the CD
    ScheduleCancel(self.show_hit_sched);
    self.show_hit_sched = schedule(self, 1500, Hash("ResetShowHit"));
    
    // -- set ourselves invulnerable
    self.invulnerable = true;
    schedule(self, 500, Hash("ResetInvulnerable"));
}

object SpawnShip()
{
    object new_ship = CreateSceneObject("Ship", '320 240 0', 20);
    return (new_ship);
}

// ====================================================================================================================
// Bullet implementation
// ====================================================================================================================
void Bullet::OnCreate()
{
    LinkNamespaces("Bullet", "SceneObject");
    SceneObject::OnCreate();
    
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

object SpawnBullet(vector3f position, vector3f direction)
{
    object bullet = CreateSceneObject("Bullet", position, 3);
    
    // -- apply the muzzle velocity
    ApplyImpulse(bullet, direction * gBulletSpeed);
    
    // -- add the bullet to the game's bullet set
    if (IsObject(gCurrentGame))
    {
        gCurrentGame.bullet_set.AddObject(bullet);
    }
}

// ====================================================================================================================
// Asteroids Game implementation
// ====================================================================================================================
void AsteroidsGame::OnCreate()
{
    LinkNamespaces("AsteroidsGame", "DefaultGame");
    DefaultGame::OnCreate();
    
    // -- create a set for the bullets (set == non-ownership)
    object self.asteroid_set = create CObjectSet("AsteroidSet");
    
    // -- create a set for the bullets (set == non-ownership)
    object self.bullet_set = create CObjectSet("BulletSet");
    
    // -- cache the 'ship' object
    object self.ship;
    
    object self.delete_set = create CObjectSet("DeleteSet");
    
}
void AsteroidsGame::OnUpdate()
{
    // -- update all the scene objects
    DefaultGame::OnUpdate();
    
    object bullet = self.bullet_set.First();
    while (IsObject(bullet))
    {
        // -- loop through all the asteroids
        bool finished = false;
        object asteroid = self.asteroid_set.First();
        while (!finished && IsObject(asteroid))
        {
            // -- if the bullet is within radius of the asteroid, it's a collision
            float distance = V3fLength(bullet.position - asteroid.position);
            if (distance < asteroid.radius)
            {
                // -- set the flag, call the function
                asteroid.OnCollision();
                
                // -- add the bullet to the delete set and break
                self.delete_set.AddObject(bullet);
                
                // $$$TZA break doesn't compile??
                finished = true;
            }
            
            // -- otherwise, get the next asteroid
            else
            {
                asteroid = self.asteroid_set.Next();
            }
        }
        
        // -- check the next bullet
        bullet = self.bullet_set.Next();
    }
    
    // -- look for asteroid collisions with the ship
    bool finished = false;
    object asteroid = self.asteroid_set.First();
    while (!finished && IsObject(asteroid))
    {
        // -- if the distance to from the ship to the asteroid < sum of their radii
        float distance = V3fLength(asteroid.position - self.ship.position);
        float max_dist = (asteroid.radius + self.ship.radius) * 0.75f;
        if (distance < max_dist)
        {
            // -- notify the ship of the collision
            self.ship.OnCollision();
            
            // -- also split the asteroid
            asteroid.OnCollision();
            
            // -- note - the asteroid could have been deleted, so the loop should exist without
            // -- continuing to iterate
            finished = true;
        }
        
        // -- else check the next asteroid
        else
        {
            asteroid = self.asteroid_set.Next();
        }
    }
    
    // -- now clean up the delete set
    while (self.delete_set.Used() > 0)
    {
        object delete_me = self.delete_set.GetObjectByIndex(0);
        destroy delete_me;
    }
}

void AsteroidsGame::OnDestroy()
{
    // -- call the default
    DefaultGame::OnDestroy();
    
    // -- clean up the extra sets
    destroy self.asteroid_set;
    destroy self.bullet_set;
    destroy self.delete_set;
}

void AsteroidsGame::OnKeyPress(int keypress)
{
    // -- rotate left
    if (keypress == CharToInt('j'))
    {
        if (IsObject(self.ship))
            self.ship.rotation -= 10.0f;
    }
    
    // -- rotate right
    else if (keypress == CharToInt('l'))
    {
        if (IsObject(self.ship))
            self.ship.rotation += 10.0f;
    }
    
    // -- thrust
    else if (keypress == CharToInt('i'))
    {
        if (IsObject(self.ship))
            self.ship.ApplyThrust(gThrust);
    }
    
    // -- fire
    else if (keypress == CharToInt(' '))
    {
        if (IsObject(self.ship))
            self.ship.OnFire();
    }
}

void StartAsteroids()
{
    ResetGame();
    gCurrentGame = create CScriptObject("AsteroidsGame");
    
    // -- spawn a ship
    gCurrentGame.ship = SpawnShip();
    
    // -- spawn, say, 8 asteroids
    schedule(0, 5000, Hash("SpawnAsteroids"));
}

void SpawnAsteroids()
{
    int i;
    for (i = 0; i < 8; i += 1)
        SpawnAsteroid();
}

// ====================================================================================================================
// EOF
// ====================================================================================================================
