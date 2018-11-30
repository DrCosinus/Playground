#include "testfile.hpp"
#include <map>

class ForwardDeclaredClass;

struct ForwardDeclaredStruct;

template<typename TYPENAME>
struct ForwardDeclaredTemplateStruct;

template<typename TYPENAME>
struct ForwardDeclaredTemplateStructWithCompleteSpecialization;

template<>
struct ForwardDeclaredTemplateStructWithCompleteSpecialization<int>
{
};

template<class CLASS>
class CompleteTemplateClass
{
};

void Blabla(ForwardDeclaredInHeader*);

struct CompleteDummyClass
{
    void MemberFunction(const ForwardDeclaredClass&);
    static void StaticMemberFunction();
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

struct Zorg : CompleteInHeader
{
};

void foo(Base* b)
{
    b->VirtualMemberFunction();
    CompleteDummyClass::StaticMemberFunction();
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