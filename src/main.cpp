#include <iostream>
#include <olc_net.h>

enum class CustomMsgTypes : uint32_t
{
    FireBullet,
    MovePlayer
};

int main()
{
#ifdef NDEBUG
    std::cout << "RELEASE CONFIG" << std::endl;
#else
    std::cout << "DEBUG CONFIG" << std::endl;
#endif

    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::FireBullet;
    std::cout << "hello" << std::endl;
    int a = 1;
    bool b = true;
    float c = 3.14159f;

    struct
    {
        float x;
        float y;
    } d[5];

    msg << a << b << c << d;

    a = 99;
    b = false;
    c = 99.0f;
    msg >> a >> b >> c >> d;

    return 0;
}