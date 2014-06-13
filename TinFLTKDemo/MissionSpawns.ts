// ====================================================================================================================
// MissionSpawns.ts
// ====================================================================================================================
Print("Executing MissionSpawns.ts");

// ====================================================================================================================
// SpawnPoint implementation
// ====================================================================================================================
void SpawnPoint::OnCreate()
{
    vector3f self.position;
}

void SpawnPoint::Initialize(vector3f pos)
{
    self.position = pos;
}

// ====================================================================================================================
// SpawnGroup implementation
// ====================================================================================================================
void SpawnGroup::OnCreate()
{
    LinkNamespaces("SpawnGroup", "CObjectSet");
    
    object self.temp_set = create CObjectSet("TempSet");
}

void SpawnGroup::OnDestroy()
{
    destroy self.temp_set;
}

void SpawnGroup::OnUpdate(float delta_time)
{
}

bool SpawnGroup::IsOccupied(object test_spawn)
{
    // -- sanity check
    if (!IsObject(test_spawn) || !IsObject(gCurrentGame) || !IsObject(gCurrentGame.character_set))
        return (false);
        
    // -- loop through the game's character set, see if anyone is within range
    vector3f spawn_pos = test_spawn.position;
    object character = gCurrentGame.character_set.First();
    while (IsObject(character))
    {
        // -- see if this character's position is too close
        if (V3fLength(spawn_pos - character.position) < (character.radius * 2.0f))
        {
            // -- occupied
            return (true);
        }
        character = gCurrentGame.character_set.Next();
    }
    
    // -- not occupied
    return (false);
}

object SpawnGroup::PickRandom()
{
    // -- find a spawn position that isn't already occupied
    object spawn_point = self.First();
    while (IsObject(spawn_point))
    {
        self.temp_set.AddObject(spawn_point);
        spawn_point = self.Next();
    }
    
    // -- pick a random number between
    while (self.temp_set.Used() > 0)
    {
        int count = self.temp_set.Used();
        int index = RandomInt(count);
        object test_spawn = self.temp_set.GetObjectByIndex(index);
        self.temp_set.RemoveObject(test_spawn);
        // $$$TZA !self.IsOccupied() isn't working
        if (self.IsOccupied(test_spawn) == false)
        {
            self.temp_set.RemoveAll();
            return (test_spawn);
        }
    }
    
    // -- apparently we were unable to find an unoccupied spawn position
    return (0);
}

object SpawnGroup::CreateSpawnPoint(vector3f position)
{
    object spawn_point = create CScriptObject("SpawnPoint");
    spawn_point.Initialize(position);
    self.AddObject(spawn_point);
}

void CornerSpawns::OnCreate()
{
    LinkNamespaces("CornerSpawns", "SpawnGroup");
    SpawnGroup::OnCreate();
}

void CreateCornerSpawnGroup()
{
    // -- create the group
    object corner_group = create CObjectGroup("CornerSpawns");
    
    if (IsObject(gCurrentGame))
        gCurrentGame.game_objects.AddObject(corner_group);
        
    // -- create the spawn points for the group
    corner_group.CreateSpawnPoint("0 0 0");
    corner_group.CreateSpawnPoint("640 0 0");
    corner_group.CreateSpawnPoint("0 480 0");
    corner_group.CreateSpawnPoint("640 480 0");
}

void EdgeSpawns::OnCreate()
{
    LinkNamespaces("EdgeSpawns", "SpawnGroup");
    SpawnGroup::OnCreate();
}

void CreateEdgeSpawnGroup()
{
    // -- create the group
    object edge_group = create CObjectGroup("EdgeSpawns");
    
    if (IsObject(gCurrentGame))
        gCurrentGame.game_objects.AddObject(edge_group);
        
    // -- create the spawn points for the group
    edge_group.CreateSpawnPoint("320 0 0");
    edge_group.CreateSpawnPoint("640 240 0");
    edge_group.CreateSpawnPoint("320 480 0");
    edge_group.CreateSpawnPoint("0 240 0");
}

// ====================================================================================================================
// SpawnWave implementation
// ====================================================================================================================
void SpawnWave::OnCreate()
{
    // -- create a set of minions that spawned by this wave
    object self.minion_set = create CObjectSet("Minions");
    
    // -- reference to the parent mission objective, for whom the wave was created
    object self.parent_objective;
    
    // -- reference to the set of spawn points used by this wave
    object self.spawn_set;
}

void SpawnWave::Initialize(object parent_objective, object spawn_set)
{
    // -- initialize the spawn wave with their parent objective, and the set of points they're to use
    self.parent_objective = parent_objective;
    self.spawn_set = spawn_set;
}

void SpawnWave::OnDestroy()
{
    // -- only the minion_set was created by this object, and must be cleaned up
    destroy self.minion_set;
}

void SpawnWave::OnKilled(object dead_minion)
{
    // -- remove the object from the set
    self.minion_set.RemoveObject(dead_minion);
    
    // -- notify the mission objective.
    if (IsObject(self.parent_objective))
    {
        self.parent_objective.NotifyOnKilled(dead_minion);
        
        // -- and to demonstrate for fun, a made up event like "last man standing"
        if (self.minion_set.Used() == 1)
            self.parent_objective.NotifyLastManStanding();
    }
}

void SpawnWave::Spawn(int count)
{
    // -- validate the object
    if (!IsObject(self.spawn_set))
        return;
        
    int i;
    for (i = 0; i < count; i += 1)
    {
        object spawn_point = self.spawn_set.PickRandom();
        if (IsObject(spawn_point))
        {
            object enemy = SpawnCharacter("Enemy", spawn_point.position);
            self.minion_set.AddObject(enemy);
        }
    }
}

object CreateSpawnWave(string spawn_set_name)
{
    object spawn_wave = create CScriptObject("SpawnWave");
    spawn_wave.Initialize(0, FindObject(spawn_set_name));
    
    // -- return the result
    return (spawn_wave);
}

// ====================================================================================================================
// EOF
// ====================================================================================================================
