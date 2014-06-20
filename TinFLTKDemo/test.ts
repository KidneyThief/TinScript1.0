LinkNamespaces("Remy", "CScriptObject");
void Remy::OnCreate()
{
    int self.coffeePercent = 0;
}

void Remy::SayHello()
{
    if (self.coffeePercent < 25)
    {
        Print("I can't even see you until I've had coffee");
    }
    else if (self.coffeePercent < 50)
    {
        Print("Still need coffee - go away");
    }
    else if (self.coffeePercent < 75)
    {
        Print("Almost ready - I need to poop now");
    }
    else
    {
        Print("Hi guys");
    }
}

void Remy::DrinkCoffee()
{
    if (self.coffeePercent < 100)
    {
        self.coffeePercent += 20;
        Print("Coffee:  ", self.coffeePercent);
        schedule(self, 5000, Hash("DrinkCoffee"));
    }
}

void Martin::SayHello()
{
    Print("Hi - don't forget unit tests!");
}

void CScriptObject::SayHello()
{
    Print("Hello!");
}

// -- Asteroids Demo
//  Run the FLTK Demo

//  DrawLine(0, "0 0 0", "320 240 0", 1);
//  DrawCircle(0, "0 0 0", "320 240 0", 2);
//  CancelDrawRequests(0);

//  Exec("asteroids.ts");
//  StartAsteroids();

//  SimPause();
//  ListObjects();

//  ship.lives = 100;

//  step 1
//  void Ship::OnSuperFire();
//  Modify OnKeyPress() - add 'q'
//  Exec("Asteroids.ts");
//  SimUnpause();

//  step 2
// AddDynamicVar(ship, "superfire_cd", "int");
// add to OnCreate() as well
//void Ship::OnSuperFire()
//{
//    int cur_time = GetSimTime();
//    self.superfire = 1500;
//}
// OnUpdate:
//    int cur_time = GetSimTime();
//    if (cur_time < self.superfire)
//    {
//        self.rotation += 10.0f;
//        self.OnFire();
//    }

//  Step 3
// 2 problems:
// *  only 4 bullets
// *  Nothing to prevent unlimited superfire!

// fix the first in OnFire:
//    int cur_time = GetSimTime();
//    if (cur_time > self.superfire_cd && gCurrentGame.bullet_set.Used() >= gMaxBullets)

// fix the second in OnSuperFire:
//    if (cur_time - self.superfire_cd > 5000)









