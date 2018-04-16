struct ParentFoo
{
    void xyz() const {};
};

struct ChildFoo : ParentFoo
{
    void xyz() const = delete;
    void quz() const {};
};

struct ParentBar
{
    virtual const ParentFoo* get() const { return &parent_foo_; }
private:
    ParentFoo parent_foo_;
};

struct ChildBar : ParentBar
{
    ChildBar(ChildFoo& _child_foo) : child_foo_{_child_foo} {}
    // changing the return type of overriden function
    // ChildFoo* is covariant to const ParentFoo*
    ChildFoo* get() const override { return &child_foo_; }
private:
    ChildFoo& child_foo_;
};

int main()
{
    ChildFoo objB;
    // ChildBar{objB}.get()->xyz(); // attempt to use a deleted function
    ChildBar{objB}.get()->quz();

    ParentBar{}.get()->xyz();
    // ParentBar{}.get()->quz(); // no member named 'quz' in 'ParentFoo'
}