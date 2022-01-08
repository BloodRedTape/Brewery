#include <string>
#include <core/os/window.hpp>
#include <unordered_map>
#include <core/function.hpp>

template<size_t SizeValue>
class InputBuffer{
private:
    char m_Data[SizeValue];
public:
    InputBuffer(){
        Clear();
    }

    size_t Size()const{
        return SizeValue;
    }

    size_t Length()const{
        return strlen(m_Data);
    }

    char *Data(){
        return m_Data;
    }

    void Clear(){
        m_Data[0] = 0;
    }
};

struct AutoWindow: Window{
    AutoWindow(int width, int height, const char *title){
        Open(width, height, title);
    }

    ~AutoWindow(){
        if(IsOpen())
            Close();
    }
};

class SimpleInterpreter{
private:
    using CommandCallback = Function<void(const char*)>;
    std::unordered_map<std::string, CommandCallback> m_Commands;
public:

    void Register(const char *name, CommandCallback callback){
        m_Commands.insert({name, callback});
    }

    bool TryInterpret(const char *string){
        std::stringstream stream(string);
        std::string command_name;
        stream >> command_name;
        auto it = m_Commands.find(command_name);
        if(it != m_Commands.end()){
            it->second(stream.str().c_str());
            return true;
        }
        return false;
    }
};