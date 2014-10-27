// --------------------------------------------------------------------------------------------------------------------
// toolpalette.ts
// Test script to demonstrate populating the debugger tool palette elements, at runtime.
// --------------------------------------------------------------------------------------------------------------------

SocketSend("ToolPaletteClear('Test Tools');");
SocketSend("ToolPaletteAddMessage('Test Tools', 'Test GUI Controls');");

// -- button
SocketSend("ToolPaletteAddButton('Test Tools', 'Test Button', 'This button says hello.', 'Press me', 'Print(`Hello there!`);');");

// -- slider
void UpdateTestSlider(int slider_value)
{
    string new_description = StringCat("`[", slider_value, "]: Update the slider value.`");
    string update_command = StringCat("ToolPaletteSetDescription(gTestSlider, ", new_description, ");");
    SocketSend(update_command);
}

SocketSend("int gTestSlider = ToolPaletteAddSlider('Test Tools', 'Test Slider', 'This slider sets an integer.', 0, 100, 50, 'UpdateTestSlider');");

// -- check box
void UpdateTestCheckBox(bool new_value)
{
    string new_description;
    if (new_value)
        new_description = StringCat("`The value is: TRUE`");
    else
        new_description = StringCat("`The value is: FALSE`");
    string update_command = StringCat("ToolPaletteSetDescription(gTestCheckBox, ", new_description, ");");
    SocketSend(update_command);
}

SocketSend("int gTestCheckBox = ToolPaletteAddCheckBox('Test Tools', 'Test CheckBox', 'Click to toggle the value.', false, 'UpdateTestCheckBox');");

// -- check box
void UpdateTestTextEdit(string new_value)
{
    string new_description = StringCat("`TextEdit contains: ", new_value, "`");
    string update_command = StringCat("ToolPaletteSetDescription(gTestTextEdit, ", new_description, ");");
    SocketSend(update_command);
}

SocketSend("int gTestTextEdit = ToolPaletteAddTextEdit('Test Tools', 'Test TextEdit', 'Enter a new string.', 'Type here', 'UpdateTestTextEdit');");
