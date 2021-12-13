#include <string>
#include <core/os/window.hpp>

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