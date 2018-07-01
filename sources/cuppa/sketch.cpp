#include "cuppa.hpp"

#include <iostream>

struct sketch : cuppa::app
{
    using cuppa::app::app;

    void setup() override
    {
        size(800, 600);
    }
    void update() override
    {
    }
    void draw() override
    {
        background(Color{ 240 });
        scale( 3 );

        pushMatrix();
        {
            translate(400, 0);

            stroke( Black );
            stroke( 5_m );
            fill( Red );
            rect({ 30_m, 30_m }, { 50_m, 50_m });

            stroke( 2_m );
            stroke( Blue );
            fill( Yellow.ModulateAlpha(127) );
            ellipse({ 55_m, 55_m }, 20_m);

            stroke(0.2_m);
            stroke( Magenta );
            line({ 5_m, 5_m}, { 55_m,55_m});
            line({ 5_m, 55_m}, { 55_m, 5_m});

            stroke( Black );
            stroke(8_m);
            point({30_m, 30_m});
        }
        popMatrix();

        pushMatrix();
        {
            translate(400 + 12.5 * 3, 250);
            stroke(1_m);
            fill( Magenta.ModulateAlpha(64) );
            arc({ 0_m, 0_m }, { 25_m, 25_m }, 0_turn, 1.25_turn, ArcMode::OPEN);
            translate(90, 0);
            arc({ 0_m, 0_m }, { 25_m, 25_m }, 0_turn, 1.25_turn, ArcMode::CHORD);
            translate(90, 0);
            arc({ 0_m, 0_m }, { 25_m, 25_m }, 0_turn, 1.25_turn, ArcMode::PIE);
        }
        popMatrix();

        pushMatrix();
        {
            translate(400, 300);
            fill( Black );
            static char buff[6] = "plop0";
            buff[4] = (((buff[4]-'0')+1)%10)+'0';
            text(buff,0,0);
        }
        popMatrix();

        noFill();
        quad({ 78_m, 31_m}, { 126_m, 20_m }, { 109_m, 53_m }, { 70_m, 66_m });

        fill( Orange.ModulateAlpha(128) );
        noStroke();
        triangle({ 30_m, 75_m}, { 58_m, 20_m}, { 86_m, 75_m });

        stroke( Black );
        fill( White);
        //rect({ 30_m + 25_m, 20_m + 25_m }, { 50_m, 50_m });
        scale(0.5);
        //rect({ 30_m + 25_m, 20_m + 25_m }, { 50_m, 50_m });

        scale(0.5);
        static float angle = 0.0f;
        shearY(PI/4);
        rotate(angle);
        angle += 0.03f;
        translate(300,300);
        rect({ 0_m, 0_m }, { 100_m, 100_m });
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}