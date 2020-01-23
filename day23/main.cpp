#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <map>
#include <array>
#include <cmath>
#include <tuple>
#include <cassert>
#include <optional>
#include <functional>
#include <stack>
#include <cassert>
#include <queue>

namespace {

    enum class OpCodes {
        STOP = 99,
        ADD = 1,
        MULTIPLY = 2,
        STORE = 3,
        OUTPUT = 4,
        JUMP_TRUE = 5,
        JUMP_FALSE = 6,
        LESS_THAN = 7,
        EQUALS = 8,
        UPDATE_BASE = 9
    };

    std::ostream& operator<<(std::ostream& s, OpCodes code) {
        switch (code) {
        case OpCodes::STOP:         s << "STOP"; break;
        case OpCodes::ADD:          s << "ADD"; break;
        case OpCodes::MULTIPLY:     s << "MULTIPLY"; break;
        case OpCodes::STORE:        s << "STORE"; break;
        case OpCodes::OUTPUT:       s << "OUTPUT"; break;
        case OpCodes::JUMP_FALSE:   s << "JUMP_FALSE"; break;
        case OpCodes::JUMP_TRUE:    s << "JUMP_TRUE"; break;
        case OpCodes::LESS_THAN:    s << "LESS_THAN"; break;
        case OpCodes::EQUALS:       s << "EQUALS"; break;
        case OpCodes::UPDATE_BASE:  s << "UPDATE_BASE"; break;
        default:                    s << std::to_string(static_cast<int64_t>(code)); break;
        }
        return s;
    }

    struct OpCode {
        OpCodes code;
        std::vector<int64_t> param_modes;
    };

    const auto printInput = [](auto& input) {
        for (auto e : input) {
            std::cout << e << ' ';
        }
        std::cout << '\n';
    };

    class IntCodeComputer {
    private:
        std::vector<int64_t> memory;
        std::optional<int64_t> onceParam;
        size_t ip{ 0 };
        int64_t relativeBase{ 0 };
        OpCodes lastOpCode{};
        bool isDebugEnabled{ true };
        bool shallOutputBreakExecution{ true };
        bool isAsciiModeEnabled{ false };

    public:
    std::queue<int> instructions;
    public:
        IntCodeComputer(std::vector<int64_t> const& mem) : memory(mem) {
            for (auto i = 0; i < 100 * 100 * 100; ++i)
                memory.push_back(0);
        }

        void setAsciiMode(bool val) {
            isAsciiModeEnabled = val;
        }

        void updateMemoryLocation(size_t pos, int64_t value) {
            memory[pos] = value;
        }

        void setMemory(std::vector<int64_t>& m) {
            memory = m;
        }

        void useOnce(int64_t param) {
            onceParam = param;
        }

        void updateInstructionPointerPosition(size_t pos) {
            ip = pos;
        }

        void enableDebug() {
            isDebugEnabled = true;
        }

        void disableDebug() {
            isDebugEnabled = false;
        }

        OpCodes getLatestCode() const {
            return lastOpCode;
        }

        void runTests(int64_t param) {
            run(param);
        }

        std::tuple<int64_t, OpCodes, int64_t> run(int64_t param) {
        //int64_t run(int64_t param) {
            bool shouldBrake = false;
            int64_t answer = 0;
            uint64_t counter = 1;

            while (true) {
                auto [opCode, mode1, mode2, mode3] = getParamModes();
                lastOpCode = opCode;
                switch (opCode) {
                case OpCodes::STOP: {
                    ++ip;
                    shouldBrake = true;
                    if (isDebugEnabled)
                        std::cout << "TERMINATE (ip: " << ip << ", opCode: " << lastOpCode << ")\n";
                    break;
                }
                case OpCodes::ADD: {
                    const auto firstParam = getValue(mode1, memory[ip + 1]);
                    const auto secondParam = getValue(mode2, memory[ip + 2]);
                    const auto pos = getIndex(mode3, memory[ip + 3]);
                    memory[pos] = firstParam + secondParam;
                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " ADD"
                        << " (" << firstParam << ", " << mode1 << ")"
                        << " (" << secondParam << ", " << mode2 << ")"
                        << " result = " << (firstParam + secondParam)
                        << " pos: " << pos
                        << '\n';
                    ip += 4;
                    break;
                }
                case OpCodes::MULTIPLY: {
                    const auto firstParam = getValue(mode1, memory[ip + 1]);
                    const auto secondParam = getValue(mode2, memory[ip + 2]);
                    const auto pos = getIndex(mode3, memory[ip + 3]);

                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " MULTIPLY"
                        << " (" << firstParam << ", " << mode1 << ")"
                        << " (" << secondParam << ", " << mode2 << ")"
                        << " result = " << (firstParam * secondParam)
                        << " pos: " << pos
                        << '\n';
                    memory[pos] = firstParam * secondParam;
                    ip += 4;
                    break;
                }
                case OpCodes::STORE: {
                    const auto pos = getIndex(mode1, memory[ip + 1]);
                    //const auto val = onceParam ? *onceParam : param;
                    const auto val = instructions.front();
                    instructions.pop();
                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " STORE"
                        << " (" << val << ", " << mode1 << ")"
                        << " pos: " << pos
                        << '\n';

                    if (onceParam) {
                        onceParam.reset();
                    }
                    memory[pos] = val;
                    ip += 2;
                    break;
                }
                case OpCodes::OUTPUT: {
                    const auto val = getValue(mode1, memory[ip + 1]);
                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " OUTPUT"
                        << " (" << val << ", " << mode1 << ")"
                        << '\n';

                    answer = val;
                    if (shallOutputBreakExecution)
                        shouldBrake = true;
                    if (isAsciiModeEnabled) {
                        std::cout << static_cast<char>(val);
                    }
                    else std::cout << val << ',' << ' ';
                    ip += 2;
                    ++counter;
                    break;
                }
                case OpCodes::JUMP_TRUE: {
                    const auto firstParam = getValue(mode1, memory[ip + 1]);
                    const auto secondParam = getValue(mode2, memory[ip + 2]);
                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " JUMP_TRUE"
                        << " (" << firstParam << ", " << mode1 << ")"
                        << " (" << secondParam << ", " << mode2 << ")"
                        << " result = " << (firstParam != 0)
                        << '\n';
                    if (firstParam != 0) {
                        ip = secondParam;
                    }
                    else
                        ip += 3;
                    break;
                }
                case OpCodes::JUMP_FALSE: {
                    const auto firstParam = getValue(mode1, memory[ip + 1]);
                    if (firstParam == 0) {
                        const auto secondParam = getValue(mode2, memory[ip + 2]);
                        if (isDebugEnabled)
                            std::cout << "ip: " << ip << " JUMP_FALSE"
                            << " (" << firstParam << ", " << mode1 << ")"
                            << " (" << secondParam << ", " << mode2 << ")"
                            << " result = " << (firstParam == 0)
                            << '\n';
                        ip = secondParam;
                    }
                    else
                        ip += 3;
                    break;
                }
                case OpCodes::LESS_THAN: {
                    const auto firstParam = getValue(mode1, memory[ip + 1]);
                    const auto secondParam = getValue(mode2, memory[ip + 2]);
                    const auto pos = getIndex(mode3, memory[ip + 3]);
                    const auto val = firstParam < secondParam;
                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " LESS_THAN"
                        << " (" << firstParam << ", " << mode1 << ")"
                        << " (" << secondParam << ", " << mode2 << ")"
                        << " result = " << (firstParam < secondParam)
                        << " pos: " << pos
                        << '\n';
                    memory[pos] = val;
                    ip += 4;
                    break;
                }
                case OpCodes::EQUALS: {
                    const auto firstParam = getValue(mode1, memory[ip + 1]);
                    const auto secondParam = getValue(mode2, memory[ip + 2]);
                    const auto pos = getIndex(mode3, memory[ip + 3]);
                    const auto val = firstParam == secondParam;

                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " EQUALS"
                        << " (" << firstParam << ", " << mode1 << ")"
                        << " (" << secondParam << ", " << mode2 << ")"
                        << " result = " << (val)
                        << '\n';
                    memory[pos] = val;
                    ip += 4;
                    break;
                }
                case OpCodes::UPDATE_BASE: {
                    const auto val = getValue(mode1, memory[ip + 1]);
                    if (isDebugEnabled)
                        std::cout << "ip: " << ip << " UPDATE_BASE"
                        << " currentVal: " << relativeBase << " updating with : " << val << ", mode:" << mode1
                        << '\n';
                    relativeBase += val;
                    ip += 2;
                    break;
                }
                }
                if (shouldBrake) break;
            }

            return std::make_tuple(answer, lastOpCode, ip);
            //return answer;
        }

    private:
        std::tuple<OpCodes, int64_t, int64_t, int64_t> getParamModes() {
            const auto value = memory[ip];
            const auto opCode = convertToOpCode(value % 100);
            const auto param1Mode = (value / 100) % 10;
            const auto param2Mode = (value / 1000) % 10;
            const auto param3Mode = (value / 10000) % 10;
            return { opCode, param1Mode, param2Mode, param3Mode };
        }

        int64_t getIndex(int64_t mode, int64_t val) {
            if (mode == 0) return val;
            if (mode == 2) return val + relativeBase;
            assert(false);
            return {};
        }

        int64_t getValue(int64_t mode, int64_t val) {
            if (mode == 0) return memory[val];                  //positional
            if (mode == 1) return val;                          //immediate
            if (mode == 2) return memory[val + relativeBase];   //relative
            assert(false);
            return {};
        }

        OpCodes convertToOpCode(int64_t code) {
            switch (code) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 99:
                return static_cast<OpCodes>(code);
            default: {
                std::cout << "ALARM! " << code << '\n';
                assert(false);
            };
            }
            return {};
        }
    };


    struct Point {
        int x;
        int y;
        int64_t type;

        bool operator<(Point const& p) const {
            if (p.x < x) return true;
            if (p.x == x) {
                return p.y < y;
            }
            return false;
        }
    };
}

int main()
{
    std::vector<int64_t> input = { 109,2050,21101,966,0,1,21102,13,1,0,1105,1,1378,21101,20,0,0,1105,1,1337,21102,27,1,0,1105,1,1279,1208,1,65,748,1005,748,73,1208,1,79,748,1005,748,110,1208,1,78,748,1005,748,132,1208,1,87,748,1005,748,169,1208,1,82,748,1005,748,239,21102,1,1041,1,21101,0,73,0,1106,0,1421,21102,78,1,1,21101,1041,0,2,21101,88,0,0,1105,1,1301,21102,1,68,1,21101,0,1041,2,21101,0,103,0,1105,1,1301,1102,1,1,750,1106,0,298,21102,1,82,1,21102,1,1041,2,21102,1,125,0,1106,0,1301,1101,2,0,750,1105,1,298,21102,79,1,1,21102,1041,1,2,21101,147,0,0,1105,1,1301,21102,1,84,1,21102,1041,1,2,21102,1,162,0,1105,1,1301,1101,3,0,750,1106,0,298,21101,65,0,1,21101,1041,0,2,21101,0,184,0,1105,1,1301,21102,1,76,1,21101,0,1041,2,21102,1,199,0,1105,1,1301,21102,1,75,1,21102,1041,1,2,21101,0,214,0,1105,1,1301,21101,221,0,0,1106,0,1337,21102,1,10,1,21101,1041,0,2,21102,236,1,0,1106,0,1301,1105,1,553,21102,1,85,1,21102,1,1041,2,21101,254,0,0,1105,1,1301,21102,78,1,1,21101,1041,0,2,21101,0,269,0,1106,0,1301,21102,276,1,0,1105,1,1337,21102,1,10,1,21101,1041,0,2,21102,291,1,0,1106,0,1301,1102,1,1,755,1105,1,553,21101,0,32,1,21101,1041,0,2,21102,313,1,0,1105,1,1301,21101,0,320,0,1105,1,1337,21102,1,327,0,1105,1,1279,1202,1,1,749,21101,0,65,2,21102,73,1,3,21102,1,346,0,1105,1,1889,1206,1,367,1007,749,69,748,1005,748,360,1102,1,1,756,1001,749,-64,751,1105,1,406,1008,749,74,748,1006,748,381,1101,0,-1,751,1106,0,406,1008,749,84,748,1006,748,395,1101,-2,0,751,1105,1,406,21102,1,1100,1,21102,406,1,0,1105,1,1421,21101,0,32,1,21102,1100,1,2,21102,1,421,0,1106,0,1301,21101,428,0,0,1106,0,1337,21102,1,435,0,1106,0,1279,1201,1,0,749,1008,749,74,748,1006,748,453,1101,-1,0,752,1105,1,478,1008,749,84,748,1006,748,467,1101,0,-2,752,1106,0,478,21101,1168,0,1,21102,478,1,0,1105,1,1421,21101,485,0,0,1106,0,1337,21102,10,1,1,21101,0,1168,2,21101,0,500,0,1106,0,1301,1007,920,15,748,1005,748,518,21102,1,1209,1,21101,518,0,0,1106,0,1421,1002,920,3,529,1001,529,921,529,1002,750,1,0,1001,529,1,537,101,0,751,0,1001,537,1,545,1001,752,0,0,1001,920,1,920,1105,1,13,1005,755,577,1006,756,570,21102,1,1100,1,21102,1,570,0,1106,0,1421,21101,987,0,1,1106,0,581,21101,1001,0,1,21102,1,588,0,1105,1,1378,1102,758,1,594,101,0,0,753,1006,753,654,21002,753,1,1,21102,610,1,0,1106,0,667,21102,1,0,1,21101,0,621,0,1105,1,1463,1205,1,647,21102,1015,1,1,21102,635,1,0,1105,1,1378,21102,1,1,1,21101,0,646,0,1106,0,1463,99,1001,594,1,594,1106,0,592,1006,755,664,1102,1,0,755,1106,0,647,4,754,99,109,2,1102,1,726,757,22101,0,-1,1,21101,0,9,2,21101,0,697,3,21101,0,692,0,1106,0,1913,109,-2,2105,1,0,109,2,1001,757,0,706,2101,0,-1,0,1001,757,1,757,109,-2,2105,1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,255,63,191,159,223,95,127,0,138,203,137,166,173,78,181,121,61,92,35,184,177,231,69,139,170,169,99,116,126,158,214,196,230,119,183,228,252,60,185,53,248,253,56,120,77,46,190,70,76,254,123,62,109,204,219,122,174,216,207,235,182,246,140,156,115,34,68,200,198,50,201,54,249,220,59,243,141,205,49,51,86,85,152,142,175,229,125,84,242,172,136,206,43,247,179,232,250,171,163,167,94,168,238,102,111,113,189,155,71,241,98,245,233,58,221,118,186,47,197,55,162,106,93,217,57,178,212,143,199,251,153,87,114,239,101,100,202,218,154,244,38,117,187,234,108,107,237,227,222,226,124,79,215,213,236,157,188,42,39,110,103,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,73,110,112,117,116,32,105,110,115,116,114,117,99,116,105,111,110,115,58,10,13,10,87,97,108,107,105,110,103,46,46,46,10,10,13,10,82,117,110,110,105,110,103,46,46,46,10,10,25,10,68,105,100,110,39,116,32,109,97,107,101,32,105,116,32,97,99,114,111,115,115,58,10,10,58,73,110,118,97,108,105,100,32,111,112,101,114,97,116,105,111,110,59,32,101,120,112,101,99,116,101,100,32,115,111,109,101,116,104,105,110,103,32,108,105,107,101,32,65,78,68,44,32,79,82,44,32,111,114,32,78,79,84,67,73,110,118,97,108,105,100,32,102,105,114,115,116,32,97,114,103,117,109,101,110,116,59,32,101,120,112,101,99,116,101,100,32,115,111,109,101,116,104,105,110,103,32,108,105,107,101,32,65,44,32,66,44,32,67,44,32,68,44,32,74,44,32,111,114,32,84,40,73,110,118,97,108,105,100,32,115,101,99,111,110,100,32,97,114,103,117,109,101,110,116,59,32,101,120,112,101,99,116,101,100,32,74,32,111,114,32,84,52,79,117,116,32,111,102,32,109,101,109,111,114,121,59,32,97,116,32,109,111,115,116,32,49,53,32,105,110,115,116,114,117,99,116,105,111,110,115,32,99,97,110,32,98,101,32,115,116,111,114,101,100,0,109,1,1005,1262,1270,3,1262,21001,1262,0,0,109,-1,2105,1,0,109,1,21102,1,1288,0,1105,1,1263,21002,1262,1,0,1102,0,1,1262,109,-1,2106,0,0,109,5,21102,1310,1,0,1106,0,1279,22101,0,1,-2,22208,-2,-4,-1,1205,-1,1332,21202,-3,1,1,21101,1332,0,0,1106,0,1421,109,-5,2105,1,0,109,2,21102,1,1346,0,1105,1,1263,21208,1,32,-1,1205,-1,1363,21208,1,9,-1,1205,-1,1363,1106,0,1373,21102,1,1370,0,1105,1,1279,1105,1,1339,109,-2,2105,1,0,109,5,1201,-4,0,1386,20102,1,0,-2,22101,1,-4,-4,21101,0,0,-3,22208,-3,-2,-1,1205,-1,1416,2201,-4,-3,1408,4,0,21201,-3,1,-3,1105,1,1396,109,-5,2106,0,0,109,2,104,10,22102,1,-1,1,21102,1,1436,0,1105,1,1378,104,10,99,109,-2,2106,0,0,109,3,20002,594,753,-1,22202,-1,-2,-1,201,-1,754,754,109,-3,2105,1,0,109,10,21101,5,0,-5,21102,1,1,-4,21102,0,1,-3,1206,-9,1555,21101,0,3,-6,21102,1,5,-7,22208,-7,-5,-8,1206,-8,1507,22208,-6,-4,-8,1206,-8,1507,104,64,1106,0,1529,1205,-6,1527,1201,-7,716,1515,21002,0,-11,-8,21201,-8,46,-8,204,-8,1106,0,1529,104,46,21201,-7,1,-7,21207,-7,22,-8,1205,-8,1488,104,10,21201,-6,-1,-6,21207,-6,0,-8,1206,-8,1484,104,10,21207,-4,1,-8,1206,-8,1569,21102,0,1,-9,1105,1,1689,21208,-5,21,-8,1206,-8,1583,21102,1,1,-9,1106,0,1689,1201,-5,716,1589,20101,0,0,-2,21208,-4,1,-1,22202,-2,-1,-1,1205,-2,1613,21201,-5,0,1,21102,1,1613,0,1106,0,1444,1206,-1,1634,22101,0,-5,1,21102,1627,1,0,1105,1,1694,1206,1,1634,21102,1,2,-3,22107,1,-4,-8,22201,-1,-8,-8,1206,-8,1649,21201,-5,1,-5,1206,-3,1663,21201,-3,-1,-3,21201,-4,1,-4,1105,1,1667,21201,-4,-1,-4,21208,-4,0,-1,1201,-5,716,1676,22002,0,-1,-1,1206,-1,1686,21101,1,0,-4,1106,0,1477,109,-10,2106,0,0,109,11,21101,0,0,-6,21102,1,0,-8,21102,1,0,-7,20208,-6,920,-9,1205,-9,1880,21202,-6,3,-9,1201,-9,921,1725,20102,1,0,-5,1001,1725,1,1733,20101,0,0,-4,21202,-4,1,1,21102,1,1,2,21102,9,1,3,21101,1754,0,0,1106,0,1889,1206,1,1772,2201,-10,-4,1767,1001,1767,716,1767,20101,0,0,-3,1106,0,1790,21208,-4,-1,-9,1206,-9,1786,21202,-8,1,-3,1106,0,1790,21201,-7,0,-3,1001,1733,1,1795,21002,0,1,-2,21208,-2,-1,-9,1206,-9,1812,22101,0,-8,-1,1106,0,1816,22101,0,-7,-1,21208,-5,1,-9,1205,-9,1837,21208,-5,2,-9,1205,-9,1844,21208,-3,0,-1,1105,1,1855,22202,-3,-1,-1,1106,0,1855,22201,-3,-1,-1,22107,0,-1,-1,1105,1,1855,21208,-2,-1,-9,1206,-9,1869,21201,-1,0,-8,1106,0,1873,21202,-1,1,-7,21201,-6,1,-6,1105,1,1708,21202,-8,1,-10,109,-11,2105,1,0,109,7,22207,-6,-5,-3,22207,-4,-6,-2,22201,-3,-2,-1,21208,-1,0,-6,109,-7,2106,0,0,0,109,5,2101,0,-2,1912,21207,-4,0,-1,1206,-1,1930,21101,0,0,-4,22102,1,-4,1,21201,-3,0,2,21102,1,1,3,21102,1949,1,0,1105,1,1954,109,-5,2105,1,0,109,6,21207,-4,1,-1,1206,-1,1977,22207,-5,-3,-1,1206,-1,1977,21201,-5,0,-5,1105,1,2045,21202,-5,1,1,21201,-4,-1,2,21202,-3,2,3,21101,0,1996,0,1106,0,1954,21201,1,0,-5,21102,1,1,-2,22207,-5,-3,-1,1206,-1,2015,21102,0,1,-2,22202,-3,-2,-3,22107,0,-4,-1,1206,-1,2037,21202,-2,1,1,21102,1,2037,0,106,0,1912,21202,-3,-1,-3,22201,-5,-3,-5,109,-6,2106,0,0
    };

    IntCodeComputer pc(input);
    pc.disableDebug();
    
    return 0;
}
