// --------------------------------------------------------------------------------------------------------------------
// toolpalette.ts
// Test script to demonstrate populating the debugger tool palette elements, at runtime.
// --------------------------------------------------------------------------------------------------------------------

SocketSend("ToolPaletteClear('Test Tools');");
SocketSend("ToolPaletteAddMessage('Test Tools', 'Test Buttons');");
SocketSend("ToolPaletteAddButton('Test Tools', 'test', 'This button says hello', 'Press me', 'Print(`Hello there!`);');");
