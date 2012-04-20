#pragma once
namespace tinia {
namespace jobobserver {
class Observer
{
public:

    virtual
    void
    finish() = 0;

    virtual
    void
    fail() = 0;

    virtual int run(int argc, char** argv) = 0;

protected:

};

}
}
