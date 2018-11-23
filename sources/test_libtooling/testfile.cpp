#include "testfile.hpp"

class ForwardDeclaredClass;

struct ForwardDeclaredStruct;

void Blabla(ForwardDeclaredInHeader*);

struct CompleteDummyClass
{
    void MemberFunction(const ForwardDeclaredClass&);
};

void FreeFunction(ForwardDeclaredClass*);

struct Base
{
    virtual void VirtualMemberFunction();
};

struct Derived : Base
{
    void VirtualMemberFunction() override;
};

struct Derived2 : Base
{
    void VirtualMemberFunction(); // No override
};

struct Derived3 : Base
{
    virtual void VirtualMemberFunction() override; // extra virtual
};

struct Derived4 : Base
{
    virtual void VirtualMemberFunction(); // No override + extra virtual
};

void foo(Base* b)
{
    b->VirtualMemberFunction();
}

void foo(Base& b)
{
    b.VirtualMemberFunction();
}

void foo()
{
    Base b;
}

void foo(int)
{
    Base* b = new Base();
}

void foo(short)
{
    auto b = new Base();
}

void foo(char)
{
    Base* b = new Base;
}

Base& GetARefToBase();

void crap()
{
    auto AutoNoRef = GetARefToBase();
    auto& AutoRef = GetARefToBase();
    auto&& AutoRefRef = GetARefToBase();
}

struct Trash
{
    Base b;
};

int addTwo(int num) 
{ 
    return num + 2; 
} 
 
int main(int, char**) 
{ 
    return addTwo(3); 
} 