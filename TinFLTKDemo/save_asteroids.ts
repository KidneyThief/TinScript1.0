void LoadObjectTree()
{

    // -- Create the objects --
    object obj_1 = create AsteroidsGame('CurrentGame');
    object obj_2 = create CObjectGroup('GameObjects');
        object obj_6 = create Ship('Ship');
        object obj_7 = create Asteroid('Asteroid');
        object obj_8 = create Asteroid('Asteroid');
        object obj_9 = create Asteroid('Asteroid');
        object obj_10 = create Asteroid('Asteroid');
        object obj_11 = create Asteroid('Asteroid');
        object obj_12 = create Asteroid('Asteroid');
        object obj_14 = create Asteroid('Asteroid');
        object obj_15 = create Bullet('Bullet');
        object obj_17 = create Asteroid('Asteroid');
        object obj_18 = create Asteroid('Asteroid');
        object obj_19 = create Bullet('Bullet');
    object obj_3 = create CObjectSet('AsteroidSet');
    object obj_4 = create CObjectSet('BulletSet');
    object obj_5 = create CObjectSet('DeleteSet');

    // -- Initialize object members --
    // -- object obj_1 member initialization
    int obj_1.sched_update = 1;
    object obj_1.game_objects = obj_2;
    int obj_1.SimTime = 50217;
    object obj_1.asteroid_set = obj_3;
    object obj_1.bullet_set = obj_4;
    object obj_1.delete_set = obj_5;
    object obj_1.ship = obj_6;

    // -- object obj_2 member initialization

    // -- object obj_6 member initialization
    vector3f obj_6.position = `294.0063 209.0210 0.0000`;
    float obj_6.radius = 20.0000;
    vector3f obj_6.velocity = `-7.7135 -9.1925 0.0000`;
    float obj_6.rotation = 230.0000;
    int obj_6.lives = 5;
    int obj_6.show_hit_sched = 0;
    bool obj_6.show_hit = false;
    vector3f obj_6.show_hit_position = `0.0000 0.0000 0.0000`;
    bool obj_6.invulnerable = false;
    float obj_6.fire_cd_time = -0.3680;

    // -- object obj_7 member initialization
    vector3f obj_7.position = `390.2817 323.0180 0.0000`;
    float obj_7.radius = 50.0000;
    vector3f obj_7.velocity = `29.2733 8.0602 0.0000`;

    // -- object obj_8 member initialization
    vector3f obj_8.position = `458.5146 160.9816 0.0000`;
    float obj_8.radius = 50.0000;
    vector3f obj_8.velocity = `27.8369 -36.7869 0.0000`;

    // -- object obj_9 member initialization
    vector3f obj_9.position = `337.5680 122.4775 0.0000`;
    float obj_9.radius = 50.0000;
    vector3f obj_9.velocity = `-19.5488 -16.4687 0.0000`;

    // -- object obj_10 member initialization
    vector3f obj_10.position = `134.9350 474.1056 0.0000`;
    float obj_10.radius = 50.0000;
    vector3f obj_10.velocity = `-1.4953 -10.4882 0.0000`;

    // -- object obj_11 member initialization
    vector3f obj_11.position = `291.9663 421.2931 0.0000`;
    float obj_11.radius = 50.0000;
    vector3f obj_11.velocity = `20.4389 29.2190 0.0000`;

    // -- object obj_12 member initialization
    vector3f obj_12.position = `65.5758 57.2565 0.0000`;
    float obj_12.radius = 50.0000;
    vector3f obj_12.velocity = `-11.2983 -33.8436 0.0000`;

    // -- object obj_14 member initialization
    vector3f obj_14.position = `305.1423 355.7585 0.0000`;
    float obj_14.radius = 50.0000;
    vector3f obj_14.velocity = `-20.0771 -2.1286 0.0000`;

    // -- object obj_15 member initialization
    vector3f obj_15.position = `173.1968 65.0460 0.0000`;
    float obj_15.radius = 3.0000;
    vector3f obj_15.velocity = `-96.4181 -114.9067 0.0000`;
    float obj_15.expireTime = 0.7830;

    // -- object obj_17 member initialization
    vector3f obj_17.position = `284.1157 170.6713 0.0000`;
    float obj_17.radius = 30.0000;
    vector3f obj_17.velocity = `-60.3906 33.9361 0.0000`;

    // -- object obj_18 member initialization
    vector3f obj_18.position = `323.7094 180.4317 0.0000`;
    float obj_18.radius = 30.0000;
    vector3f obj_18.velocity = `24.2113 54.7918 0.0000`;

    // -- object obj_19 member initialization
    vector3f obj_19.position = `239.6367 144.2259 0.0000`;
    float obj_19.radius = 3.0000;
    vector3f obj_19.velocity = `-96.4181 -114.9067 0.0000`;
    float obj_19.expireTime = 1.5320;

    // -- object obj_3 member initialization

    // -- object obj_4 member initialization

    // -- object obj_5 member initialization


    // -- Restore object hierarchy --
    obj_2.AddObject(obj_6);
    obj_2.AddObject(obj_7);
    obj_2.AddObject(obj_8);
    obj_2.AddObject(obj_9);
    obj_2.AddObject(obj_10);
    obj_2.AddObject(obj_11);
    obj_2.AddObject(obj_12);
    obj_2.AddObject(obj_14);
    obj_2.AddObject(obj_15);
    obj_2.AddObject(obj_17);
    obj_2.AddObject(obj_18);
    obj_2.AddObject(obj_19);
    obj_3.AddObject(obj_7);
    obj_3.AddObject(obj_8);
    obj_3.AddObject(obj_9);
    obj_3.AddObject(obj_10);
    obj_3.AddObject(obj_11);
    obj_3.AddObject(obj_12);
    obj_3.AddObject(obj_14);
    obj_3.AddObject(obj_17);
    obj_3.AddObject(obj_18);
    obj_4.AddObject(obj_15);
    obj_4.AddObject(obj_19);
}

schedule(0, 1, hash('LoadObjectTree'));
