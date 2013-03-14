
// ------------------------------------------------------------------------------------------------
void CWeapon::OnCreate() {
    Print("CWeapon::OnCreate()");
    self.readytofire = true;
}

void CWeapon::Fire() {
    if(self.readytofire)
        Print("FIRE!");
    else
        Print("Reloading!");
}

int gUpdateWeaponsThread = 0;
void UpdateWeapons() {
    //Print("Updating weapons...");
    UpdateWeaponList();
    gUpdateWeaponsThread = schedule(0, 10, UpdateWeapons);
}

// ------------------------------------------------------------------------------------------------
void Pistol::OnCreate() {
    Print("Pistol::OnCreate()");
    LinkNamespaces("Pistol", "CWeapon");
    CWeapon::OnCreate();
}

void Pistol::OnUpdate() {
    //Print("Pistol::OnUpdate()");
}

// ------------------------------------------------------------------------------------------------
void Bazooka::OnCreate() {
    Print("Bazooka::OnCreate()");
    LinkNamespaces("Bazooka", "CWeapon");
    CWeapon::OnCreate();
    int self.reloadtimer = 0;
}

void Bazooka::OnUpdate() {
    //Print("Bazooka::OnUpdate()");
}

void Bazooka::Fire() {
    if(self.readytofire) {
        CWeapon::Fire();
        self.readytofire = false;
        schedule(self, 3000, Reload);
    }
    else {
        Print("Reloading!");
    }
}

void Bazooka::Reload() {
    self.readytofire = true;
}

// ------------------------------------------------------------------------------------------------
void Thermite::OnCreate() {
    Print("Thermite::OnCreate()");
    LinkNamespaces("Thermite", "CWeapon");
    CWeapon::OnCreate();
    
    // -- need a state variable, and a timer
    int self.firestate = 0;
    float self.firetimer = 0.0f;
}

int gThermiteStateCharge = 1;
int gThermiteStateFire = 2;
int gThermiteStateCoolDown = 3;

int Thermite::GetState() {
    int cur_state = self.firestate;
    return (cur_state);
}

void Thermite::OnUpdate() {
    //Print("Thermite::OnUpdate()");
    float curtime = GetSimTime();
    
    // -- get our current state
    int cur_state = self.GetState();
    
    // -- see if we're done charging
    if(cur_state == gThermiteStateCharge) {
        if(curtime >= self.firetimer) {
            Print("Thermite Spraying...");
            self.firestate = gThermiteStateFire;
            self.firetimer = curtime + 2.0f;
        }
    }
    else if(cur_state == gThermiteStateFire) {
        if(curtime >= self.firetimer) {
            Print("Thermite Cooling...");
            self.firestate = gThermiteStateCoolDown;
            self.firetimer = curtime + 5.0f;
        }
    }
    else if(cur_state == gThermiteStateCoolDown) {
        if(curtime >= self.firetimer) {
            Print("Thermite Ready");
            self.firestate = 0;
            self.firetimer = 0.0f;
            self.readytofire = true;
        }
    }
}

void Thermite::Fire() {
    Print("Entering Thermite::Fire()");
    if(self.readytofire) {
        Print("Thermite Charging...");
        self.readytofire = false;
        self.firestate = gThermiteStateCharge;
        self.firetimer = GetSimTime() + 3.0f;
    }
    else {
        Print("Thermite::Fire() still reloading!");
    }
}

void Thermite::Reload() {
    self.readytofire = true;
}

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
