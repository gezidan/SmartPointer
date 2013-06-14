#define MySP    1


#include <iostream>
#include <memory>
#if MySP
#include "smart_ptr.h"
#endif

class CBase
{
public:
    CBase() : data0(0)
    {
        std::cout << "CBase()" << std::endl;
    }

    virtual ~CBase()
    {
        std::cout << "~CBase()" << std::endl;
    }

    int data0;
private:
};

class CDevide : public CBase
{
public:
    CDevide() : data2(3)
    {
        std::cout << "CDevide()" << std::endl;
    }

    ~CDevide()
    {
        std::cout << "~CDevide()" << std::endl;
    }

    int data2;
private:
};

//std::shared_ptr;
//std::weak_ptr;

int main(void)
{
#if MySP
    strong_ptr<CBase> spDev(new CDevide());

    {
        strong_ptr<CDevide> sp2; sp2 = (spDev);
        std::cout << sp2->data0 << std::endl;
    }
#else
    std::shared_ptr<CDevide> spDev(new CDevide());

    {
        std::shared_ptr<CBase> sp2(spDev);
        std::cout << sp2->data0 << std::endl;

        std::make_shared();
    }
#endif

    return 0;
}
