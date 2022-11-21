#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include <thread>
#include <vector>

using namespace sf;
using namespace std;

const float PI = 3.14159265358979323846f;
int WIDTH = 1200;
int HEIGHT = 720;

#include "../../Libraries/Library.h"
Camera cam = Camera(Vector2f(0, 0));
Camera3d cam3d = Camera3d(Vector3ff(3, -1, 13), 0.8f);

float maxFPS = 60;
float deltaTime = 0;
Clock frameStartTime;
Text FPSDisplay;
Font defaultFont;
Clock fpsDisplayUpdate;

Vector2i mousePosition, oldMousePosition;
bool isMouseClicked;

bool isKeyboardClicked;
Keyboard::Key keyClicked;

float a = 0.9f;

const int gridSize = 90;

class Grid
{
private:
    vector<Vertex> grid;
    vector<vector<float>> pastPoints;
    vector<vector<float>> pastPastPoints;

public:
    vector<vector<float>> points;
    int size = 10;
    Grid(int size = 100)
    {
        this->size = size;
        for (int i = 0; i < size; i++)
        {
            vector<float> row;
            vector<float> pastRow;
            for (int j = 0; j < size; j++)
            {
                // row.push_back((sinf((float)i/size*2*PI*3)+sinf((float)i/size*2*PI))*0.0);//sinf((float)i/size*2*PI*3)*0.8
                // row.push_back((sinf((float)i / size * 2 * PI * 3) + sinf((float)i / size * 2 * PI)) * 0.0002f);
                row.push_back(0.0f);
                pastRow.push_back(0);
                // row.push_back(0);
            }
            this->points.push_back(row);
            this->pastPoints.push_back(pastRow);
            this->pastPastPoints.push_back(pastRow);
        }
        for (float i = 0; i < size; i++)
        {
            for (float j = 0; j < size; j++)
            {
                Color clr = Color(50, 50, 0);
                this->grid.push_back(Vertex(Vector2f(j / size * WIDTH, i / size * HEIGHT), clr));
                this->grid.push_back(Vertex(Vector2f((j + 1) / size * WIDTH, i / size * HEIGHT), clr));
                this->grid.push_back(Vertex(Vector2f((j + 1) / size * WIDTH, (i + 1) / size * HEIGHT), clr));
                this->grid.push_back(Vertex(Vector2f(j / size * WIDTH, (i + 1) / size * HEIGHT), clr));
            }
        }
        cout << 4 * size * size << endl;
        cout << this->grid.size() << endl;
    }
    void updateColors()
    {
        // cout << this->grid.getVertexCount(); << endl;
        for (int i = 0; i < this->grid.size(); i++)
        {
            int pi = (int)floor((float)i / 4 / this->size);
            int pj = (int)((i / 4) % this->size);

            this->grid[i].color = Color((int)(this->points[pi][pj] * 255), 0, -(int)(this->points[pi][pj] * 255));
            // this->grid[i].color = Color::Red;
        }
    }
    void Draw(RenderWindow *window)
    {
        // this->updateColors();
        window->draw(&this->grid[0], this->grid.size(), Quads);
    }

    float getPointOf(vector<vector<float>> *tPoints, int i, int j, int vi = 0, int vj = 0)
    {
        int i0 = i;
        int j0 = j;

        if (i0 < 0)
            i0 = tPoints->size() + i0;
        if (j0 < 0)
            j0 = tPoints->size() + j0;
        if (i0 > size - 1)
            i0 = i0 % (tPoints->size());
        if (j0 > size - 1)
            j0 = j0 % (tPoints->size());

        return (*tPoints)[i0][j0];
    }
    vector<vector<float>> getNewPoints(Vector2i AreaMin, Vector2i AreaMax)
    {
        vector<vector<float>> newPoints;
        for (int i = AreaMin.x; i < AreaMax.x; i++)
        {
            vector<float> row;
            for (int j = AreaMin.y; j < AreaMax.y; j++)
            {
                float neighborSum = this->getPointOf(&this->points, i + 1, j) + this->getPointOf(&this->points, i - 1, j) + this->getPointOf(&this->points, i, j + 1) + this->getPointOf(&this->points, i, j - 1) - 4 * this->getPointOf(&this->points, i, j);
                row.push_back(a * a * neighborSum / 2 + 2 * this->getPointOf(&this->points, i, j) - this->getPointOf(&this->pastPoints, i, j));

                // float v = -3 * this->getPointOf(&this->points, i, j) * (this->getPointOf(&this->points, i + 1, j) - this->getPointOf(&this->points, i, j)) + 2 * this->getPointOf(&this->points, i, j);
                // v += -(3 * this->getPointOf(&this->pastPastPoints, i, j) + this->getPointOf(&this->pastPoints, i, j)) / 2;
                // row.push_back(v);
            }
            newPoints.push_back(row);
        }
        return newPoints;
    }
    void applyNewPoints(vector<vector<float>> newPoints, Vector2i StartPoint)
    {
        for (int i = 0; i < newPoints.size(); i++)
        {
            for (int j = 0; j < newPoints.size(); j++)
            {
                this->pastPastPoints[StartPoint.x + i][StartPoint.y + j] = this->pastPoints[StartPoint.x + i][StartPoint.y + j];
            }
        }
        for (int i = 0; i < newPoints.size(); i++)
        {
            for (int j = 0; j < newPoints.size(); j++)
            {
                this->pastPoints[StartPoint.x + i][StartPoint.y + j] = this->points[StartPoint.x + i][StartPoint.y + j];
            }
        }
        for (int i = 0; i < newPoints.size(); i++)
        {
            for (int j = 0; j < newPoints[i].size(); j++)
            {
                // this->points[StartPoint.x+i][StartPoint.y+j] = newPoints[StartPoint.x+i][StartPoint.y+j]*0.98;
                this->points[StartPoint.x + i][StartPoint.y + j] = newPoints[i][j] * 0.975f;
            }
        }
    }
};

Plane visualPlane = Plane(Vector3ff(0, 0, 0), Vector2i(gridSize, gridSize), Vector2f(10, 10));

Grid grid = Grid(gridSize);

int frame = 0;

int main()
{
    cout << "Start";
    ContextSettings settings;
    settings.antialiasingLevel = 1;
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "SFML window", Style::Default, settings);

    defaultFont = Font();
    defaultFont.loadFromFile("E:\\!Coding\\c++\\Projects\\SFML\\Default\\sansation.ttf");

    FPSDisplay.setFillColor(Color::Green);
    FPSDisplay.setFont(defaultFont);
    FPSDisplay.setCharacterSize(16);
    fpsDisplayUpdate = Clock();

    frameStartTime = Clock();

    visualPlane.color = Color(0, 156, 228);
    visualPlane.GenerateGrid();

    cam3d.Rotate(Vector3ff(0, PI, 0));

    while (window.isOpen())
    {
        WIDTH = window.getSize().x;
        HEIGHT = window.getSize().y;
        deltaTime = frameStartTime.getElapsedTime().asSeconds();
        // if (deltaTime < 1 / maxFPS)
        //     continue;
        frameStartTime.restart();

        if (fpsDisplayUpdate.getElapsedTime().asSeconds() > 0.12f)
        {
            FPSDisplay.setString(to_string((int)round(1 / deltaTime)));
            fpsDisplayUpdate.restart();
        }

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::MouseButtonPressed)
            {
                isMouseClicked = true;
            }
            if (event.type == Event::MouseButtonReleased)
            {
                isMouseClicked = false;
            }
            if (event.type == Event::MouseMoved)
            {
                mousePosition = Mouse::getPosition(window);
                if (isMouseClicked)
                {
                    Vector2i delta = mousePosition - oldMousePosition;
                    cam.position += (Vector2f)delta;
                }
                if (Mouse::isButtonPressed(Mouse::Button::Right))
                {
                    cam3d.Rotate(Vector3ff(cam3d.rotation.x - (mousePosition.x - oldMousePosition.x) * 0.001f, cam3d.rotation.y - (mousePosition.y - oldMousePosition.y) * 0.001f, cam3d.rotation.z));
                }
                oldMousePosition = mousePosition;
            }
            if (event.type == Event::KeyPressed)
            {
                keyClicked = event.key.code;
                isKeyboardClicked = true;
            }
            if (event.type == Event::KeyReleased)
            {
                isKeyboardClicked = false;
            }
            if (event.type == Event::MouseWheelScrolled)
            {
                cam.scale += event.mouseWheel.delta;
            }
        }
        if (isKeyboardClicked)
        {
            float speed = 10 * deltaTime;
            if (keyClicked == Keyboard::A)
            {
                cam3d.position += cam3d.right * -1.0f * speed;
            }
            if (keyClicked == Keyboard::W)
            {
                cam3d.position += cam3d.forward * speed;
            }
            if (keyClicked == Keyboard::S)
            {
                cam3d.position += cam3d.forward * -1.0f * speed;
            }
            if (keyClicked == Keyboard::D)
            {
                cam3d.position += cam3d.right * speed;
            }
            if (keyClicked == Keyboard::R)
            {
                cam3d.position += cam3d.up * speed;
            }
            if (keyClicked == Keyboard::F)
            {
                cam3d.position += cam3d.up * -1.0f * speed;
            }
        }

        if (isMouseClicked && Mouse::isButtonPressed(Mouse::Button::Left))
        {
            int i0 = (int)(((float)Mouse::getPosition(window).y / HEIGHT) * grid.size);
            int j0 = (int)(((float)Mouse::getPosition(window).x / WIDTH) * grid.size);

            for (int i = 0; i < grid.size; i++)
            {
                for (int j = 0; j < grid.size; j++)
                {
                    grid.points[i][j] += (0.3f * sinf((float)frame * 0.4)) / (1.0f + (powf(i0 - i, 2) + powf(j0 - j, 2)) * 0.15);
                }
            }
        }
        // int XP = (cosf(frame * 0.1) * 0.1 + 1) / 2 * grid.size;
        // int YP = (sinf(frame * 0.1) * 0.1 + 1) / 2 * grid.size;
        // int XP2 = (cosf(frame * 0.1 + PI) * 0.1 + 1) / 2 * grid.size;
        // int YP2 = (sinf(frame * 0.1 + PI) * 0.1 + 1) / 2 * grid.size;
        // for (int i = 0; i < grid.size; i++)
        // {
        //     for (int j = 0; j < grid.size; j++)
        //     {
        //         // grid.points[i][j] += 0.2f / (1.0f + (powf(XP - i, 2) + powf(YP - j, 2)) * 0.2);
        //         // grid.points[i][j] += -0.2f / (1.0f + (powf(XP2 - i, 2) + powf(YP2 - j, 2)) * 0.2);
        //     }
        // }

        Clock c1 = Clock();
        grid.applyNewPoints(grid.getNewPoints(Vector2i(0, 0), Vector2i(gridSize, gridSize)), Vector2i(0, 0));
        // cout << c1.getElapsedTime().asMilliseconds() << endl;

        visualPlane.SetGridByArray(grid.points);

        visualPlane.Draw(&window, &cam3d);

        window.draw(FPSDisplay);
        window.display();
        window.clear();
        frame++;
    }
    return 0;
}