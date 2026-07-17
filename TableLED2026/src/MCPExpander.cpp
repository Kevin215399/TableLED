#include "TableLED.h"

namespace MCP {
byte data[4][11];
void (*InterruptFunctions[4][7])(MCPInterruptData);
}


const byte ADDRESSES[] = {0x20, 0x21, 0x22, 0x23};

const byte REGISTER_DEFAULTS[] = {
    255,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};



bool interruptFlag = false;

void MCP::Start()
{
    Wire.begin();

    for (int i = 0; i < MCP_COUNT; i++)
    {
        Wire.beginTransmission(ADDRESSES[i]);

        // Set register index to 0
        Wire.write(IODIR);

        // Reset registers
        for (int a = 0; a < 11; a++)
        {
            // MCP23008 auto increments register pointer
            Wire.write(REGISTER_DEFAULTS[a]);
        }
        Wire.endTransmission();

        if (i != 0)
        {
            Wire.beginTransmission(ADDRESSES[i]);
            Wire.write(GPINTEN);
            Wire.write((1 << 0) | data[i][GPINTEN]);
            Wire.endTransmission();
        }

        SyncReg(i);
    }

    attachInterrupt(REG_INTERRUPT, InterruptHandler, FALLING);
}

void MCP::SyncReg(byte mcpIndex)
{
    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(IODIR);
    Wire.endTransmission(false);

    Wire.requestFrom(ADDRESSES[mcpIndex], 11);

    byte regIndex = IODIR;
    while (Wire.available())
    {
        if (regIndex >= 11)
        {
            Wire.read();
            continue;
        }
        data[mcpIndex][regIndex++] = Wire.read();
    }
}

void MCP::PinMode(byte mcpIndex, byte pin, byte mode)
{
    if (mode != INPUT && mode != OUTPUT && mode != INPUT_PULLUP)
        return;

    SyncReg(mcpIndex);

    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(IODIR);

    switch (mode)
    {
    case INPUT:
    case INPUT_PULLUP:
        Wire.write((1 << pin) | data[mcpIndex][IODIR]);
        break;
    case OUTPUT:
        Wire.write(~(1 << pin) & data[mcpIndex][IODIR]);
        break;
    default:
        break;
    }

    Wire.endTransmission();

    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(GPPU);
    if (mode == INPUT_PULLUP)
    {
        Wire.write((1 << pin) | data[mcpIndex][GPPU]);
    }
    else
    {
        Wire.write(~(1 << pin) && data[mcpIndex][GPPU]);
    }
    Wire.endTransmission();

    SyncReg(mcpIndex);
}

bool MCP::Read(byte mcpIndex, byte pin)
{
    SyncReg(mcpIndex);
    return (data[mcpIndex][GPIO] & (1 << pin)) > 0;
}

void MCP::Write(byte mcpIndex, byte pin, bool state)
{
    SyncReg(mcpIndex);
    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(OLAT);
    if (state)
    {
        Wire.write((1 << pin) | data[mcpIndex][OLAT]);
    }
    else
    {
        Wire.write(~(1 << pin) && data[mcpIndex][OLAT]);
    }
    Wire.endTransmission();
}

void MCP::Interrupt(byte mcpIndex, byte pin, void (*function)(MCPInterruptData))
{
    SyncReg(mcpIndex);

    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(GPINTEN);
    Wire.write((1 << pin) | data[mcpIndex][GPINTEN]);
    Wire.endTransmission();

    InterruptFunctions[mcpIndex][pin] = function;
}

void MCP::Interrupt(byte mcpIndex, byte pin, void (*function)(MCPInterruptData), int mode)
{
    SyncReg(mcpIndex);

    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(GPINTEN);
    Wire.write((1 << pin) | data[mcpIndex][GPINTEN]);
    Wire.endTransmission();

    switch (mode)
    {
    case RISING:
        Wire.beginTransmission(ADDRESSES[mcpIndex]);
        Wire.write(DEFVAL);
        Wire.write(~(1 << pin) & data[mcpIndex][DEFVAL]);
        Wire.write((1 << pin) | data[mcpIndex][INTCON]);
        Wire.endTransmission();
        break;
    case FALLING:
        Wire.beginTransmission(ADDRESSES[mcpIndex]);
        Wire.write(DEFVAL);
        Wire.write((1 << pin) | data[mcpIndex][DEFVAL]);
        Wire.write((1 << pin) | data[mcpIndex][INTCON]);
        Wire.endTransmission();
        break;
    default:
        break;
    }

    InterruptFunctions[mcpIndex][pin] = function;
}

void MCP::NoInterrupt(byte mcpIndex, byte pin)
{
    SyncReg(mcpIndex);

    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(GPINTEN);
    Wire.write(~(1 << pin) & data[mcpIndex][GPINTEN]);
    Wire.endTransmission();

    Wire.beginTransmission(ADDRESSES[mcpIndex]);
    Wire.write(DEFVAL);
    Wire.write(~(1 << pin) & data[mcpIndex][DEFVAL]);
    Wire.write(~(1 << pin) & data[mcpIndex][INTCON]);
    Wire.endTransmission();

    InterruptFunctions[mcpIndex][pin] = NULL;
}

void MCP::ARDUINO_ISR_ATTR InterruptHandler()
{
    xTaskCreatePinnedToCore(
        CallInterruptFunction,
        "interrupt",
        10000,
        NULL,
        1,
        NULL,
        1);
}

void MCP::CallInterruptFunction(void *pv)
{
    Serial.println("Interrupt called");
    // get the flagged mcp
    for (int m = MCP_COUNT - 1; m >= 0; m--)
    {
        bool flaggedMCP = false;
        SyncReg(m);
        if (m == 0)
        {
            // MCP 0 has pin 0 free, so it can also detect interrupts
            if (data[0][INTF] > 0)
            {
                flaggedMCP = true;
                Serial.println("Interrupt 0");
            }
        }
        else
        {
            if ((data[m][INTF] & (~1)) > 0)
            {
                flaggedMCP = true;
                Serial.print("Interrupt ");
                Serial.println(m);
            }
        }

        

        if (!flaggedMCP)
            continue;

        MCPInterruptData interruptData;
        byte firstPin = 1;
        if (m == 0)
        {
            firstPin = 0;
        }
        // loop through pins
        for (int i = firstPin; i < 8; i++)
        {

            if ((data[m][INTF] & (1 << i)) == 0)
                continue;
            if (InterruptFunctions[m][i] == NULL)
                continue;
            Serial.print("pin ");
            Serial.println(i);
            interruptData.MCP = m;
            interruptData.pin = i;
            interruptData.state = data[m][INTCAP];
            // call the function
            InterruptFunctions[m][i](interruptData);
        }
    }

    
    vTaskDelete(NULL);
}