#include <iostream>

struct Bar
{
  enum class Visibility : char { Visible, Hidden };
  enum class ShowImpact : char { Show, DoNotShow };
  enum class ApplyDamage : char { Apply, DoNotApply };
  enum class SpawnSideEffect : char { Spawn, DoNotSpawn };
  // argument names are not necessary
  void ProcessImpact(Visibility, ShowImpact, ApplyDamage, SpawnSideEffect)
  {
  }
};

struct Foo
{
  enum class Visibility : char { Hidden, Visible }; // please that Hidden and Visible and in the reverse order compared to Bar::Visible
  Visibility myVisibility; // no comment needed
  Bar::Visibility myBarVisibility; // no comment needed
  Bar::ShowImpact myShowImpact;
  Bar::ApplyDamage myApplyDamage;
};

template<typename ToEnum, typename FromEnum>
ToEnum ConvertTo(FromEnum);

template<>
Foo::Visibility ConvertTo<Foo::Visibility>(Bar::Visibility aBarEnum)
{
    return (aBarEnum == Bar::Visibility::Visible) ? Foo::Visibility::Visible : Foo::Visibility::Hidden;
}

template<>
Bar::Visibility ConvertTo<Bar::Visibility>(Foo::Visibility aFooEnum)
{
    return (aFooEnum == Foo::Visibility::Visible) ? Bar::Visibility::Visible : Bar::Visibility::Hidden;
}

template<typename T>
const char* ToString(const T&);

template<>
const char* ToString<Bar::Visibility>(const Bar::Visibility& aBarVisibility)
{
    return aBarVisibility==Bar::Visibility::Hidden ? "Bar::Visibility::Hidden" : "Bar::Visibility::Visible";
}

int main()
{
  auto bar = Bar{};
  auto foo = Foo{};

  //bar.ProcessImpact(foo.myVisibility, foo.myApplyDamage, foo.myShowImpact, true);
  // do not compile because:
  // - first param:  argument of type 'Foo::Visibility' is incompatible with parameter of type 'Bar::Visibility'
  // - second param: argument of type 'Foo::ApplyDamage' is incompatible with parameter of type 'Bar::ShowImpact'
  // - third param:  argument of type 'Foo::ShowImpact' is incompatible with parameter of type 'Bar::ApplyDamage'
  // - fourth param: argument of type 'bool' is incompatible with parameter of type 'Bar::ApplyDamage'

  // OK
  bar.ProcessImpact(foo.myBarVisibility, foo.myShowImpact, foo.myApplyDamage, Bar::SpawnSideEffect::Spawn);

  // if the user intentionnally want to pass a Foo::Visibility for a Bar::Visibility
  // he/she has to explicit the "conversion"
  bar.ProcessImpact(ConvertTo<Bar::Visibility>(foo.myVisibility), foo.myShowImpact, foo.myApplyDamage, Bar::SpawnSideEffect::Spawn);

  std::cout << ToString(Bar::Visibility::Hidden) << std::endl;
  std::cout << ToString(Bar::Visibility::Visible) << std::endl;
}