

#define IODIR 0
#define IPOL 1
#define GPINTEN 2
#define DEFVAL 3
#define INTCON 4
#define IOCON 5
#define GPPU 6
#define INTF 7
#define INTCAP 8
#define GPIO 9
#define OLAT 10

#define MCP_COUNT 4
#define REG_INTERRUPT 27

struct MCPInterruptData
{
    byte MCP;
    byte pin;
    bool state;
};

extern const byte ADDRESSES[];
extern const byte REGISTER_DEFAULTS[];

namespace MCP
{
    extern byte data[4][11];
    extern void (*InterruptFunctions[4][7])(MCPInterruptData);

    bool Read(byte mcpIndex, byte pin);
    void Start();
    void SyncReg(byte mcpIndex);
    void PinMode(byte mcpIndex, byte pin, byte mode);
    void Write(byte mcpIndex, byte pin, bool state);
    void Interrupt(byte mcpIndex, byte pin, void (*function)(MCPInterruptData));
    void Interrupt(byte mcpIndex, byte pin, void (*function)(MCPInterruptData), int mode);
    void NoInterrupt(byte mcpIndex, byte pin);
    void ARDUINO_ISR_ATTR InterruptHandler();
    void CallInterruptFunction(void *pv);
};
