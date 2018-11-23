#include "testfile.hpp"

class ForwardDeclaredClass;

struct ForwardDeclaredStruct;

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

int addTwo(int num) 
{ 
    return num + 2; 
} 
 
int main(int, char**) 
{ 
    return addTwo(3); 
} 