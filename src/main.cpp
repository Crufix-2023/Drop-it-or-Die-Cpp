/////////////////////

#include "includes.h"

/////////////////////

using namespace std;
using namespace tgui;
using namespace sf;

/////////////////////
 
// variables
static short score_pl1_short = 0, score_pl2_short = 0, \
             number_cube1_pl1, number_cube1_pl2, \
             number_cube2_pl1, number_cube2_pl2, \
             number_cubes_pl1, number_cubes_pl2;

static string pl1_name = "pl1", pl2_name = "pl2", full_text_1, full_text_2;

string who_win_mass[] = { "pl1 win", "pl2 win", "draw" }, who_win;


/* timer()
вызов таймера: timer(2.9); | где timer() - функция, а 2.9 - время в секундах
*/

/* randi()
вызов рандома int чисел: randi(abc, 0, 1001); | где randi() - функция, abc - существующая переменная в которую записывается рандомное число,
                                              | 0 - минимальное число, а 1001 - максимальное.
                                              | переменную и минимальные/максимальные числа можно вписывать любые - 
                                              | - (для randi только целые числа)
*/

/* randsh()
вызов рандома short чисел: randi(abc, 0, 1001); | randsh() - функция, abc - существующая переменная в которую записывается рандомное число,
                                                | 0 - минимальное число, а 1001 - максимальное.
                                                | переменную и минимальные/максимальные числа можно вписывать любые -
                                                | - (для randsh только целые числа)
*/

/* randf()
вызов рандома float чисел: randf(abc, 0.46, 1001.21); | где randf() - функция, abc - существующая переменная в которую записывается рандомное число,
                                                      | 0.46 - минимальное число, а 1001.21 - максимальное.
                                                      | переменную и минимальные/максимальные числа можно вписывать любые -
                                                      | - (для randf можно и целые числа, и числа с плавающей точкой)
*/


void timer(double seconds) {
    std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
}

void randi(int& result, int one, int two) {
    random_device rd;
    mt19937 rand(rd());

    uniform_int_distribution<int> dist(one, two);

    result = dist(rand);
}
void randsh(short& result, short one, short two) {
    random_device rd;
    mt19937 rand(rd());

    uniform_int_distribution<short> dist(one, two);

    result = dist(rand);
}
void randf(float& result, float one, float two) {
    random_device rd;
    mt19937 rand(rd());

    uniform_real_distribution<float> dist(one, two);

    result = dist(rand);
}


void play() {
    randsh(number_cube1_pl1, 1, 6);
    randsh(number_cube2_pl1, 1, 6);

    randsh(number_cube1_pl2, 1, 6);
    randsh(number_cube2_pl2, 1, 6);

    number_cubes_pl1 = number_cube1_pl1 + number_cube2_pl1;
    number_cubes_pl2 = number_cube1_pl2 + number_cube2_pl2;

    if (number_cubes_pl1 > number_cubes_pl2) {
        who_win = who_win_mass[0];
    }
    if (number_cubes_pl1 < number_cubes_pl2) {
        who_win = who_win_mass[1];
    }
    if (number_cubes_pl1 == number_cubes_pl2) {
        who_win = who_win_mass[2];
    }

    cout << "pl1: " << number_cubes_pl1 << " | pl2: " << number_cubes_pl2 << " == " << who_win << "\n--------------------------------\n" << endl;
}


int main() {
    const float originalWidth = 1024.0f;
    const float originalHeight = 512.0f;
    
    RenderWindow window{VideoMode{ {static_cast<unsigned int>(originalWidth), static_cast<unsigned int>(originalHeight)} }, "Drop it or Die"};
    window.setFramerateLimit(60);
    Gui gui{window};

    auto theme = tgui::Theme::create("./assets/themes/theme.txt");
    auto font = "./assets/fonts/Hero-Bold.ttf";

    auto texture_hand_pl1 = tgui::Texture("./assets/textures/game/hands/hand_blue.png");
    auto texture_hand_pl2 = tgui::Texture("./assets/textures/game/hands/hand_red.png");
    auto glass_up = tgui::Texture("./assets/textures/game/cup/glass_up.png");
    auto glass_down = tgui::Texture("./assets/textures/game/cup/glass_down.png");

    // Исходные позиции в пикселях от 1024x512
    struct WidgetPosition {
        float x, y, width, height;
    };

    WidgetPosition positions[] = {
        {558, 426, 150, 150},  // cup_pl1
        {467, 86, 150, 150},   // cup_pl2
        {372, 446, 100, 100},  // left_hand_pl1
        {652, 451, 100, 100},  // right_hand_pl1
        {652, 106, 100, 100},  // left_hand_pl2
        {372, 109, 100, 100},  // right_hand_pl2
        {512, 256, 150, 70}    // btn_tap
    };


    // Widgets
    auto cup_pl1 = tgui::Picture::create(glass_up); gui.add(cup_pl1);
    cup_pl1->setOrigin(0.5, 0.5);

    auto cup_pl2 = tgui::Picture::create(glass_up); gui.add(cup_pl2);
    cup_pl2->setOrigin(0.5, 0.5);

    auto left_hand_pl1 = tgui::Picture::create(texture_hand_pl1); gui.add(left_hand_pl1);
    left_hand_pl1->setOrigin(0.5, 0.5);

    auto right_hand_pl1 = tgui::Picture::create(texture_hand_pl1); gui.add(right_hand_pl1);
    right_hand_pl1->setOrigin(0.5, 0.5);

    auto left_hand_pl2 = tgui::Picture::create(texture_hand_pl2); gui.add(left_hand_pl2);
    left_hand_pl2->setOrigin(0.5, 0.5);

    auto right_hand_pl2 = tgui::Picture::create(texture_hand_pl2); gui.add(right_hand_pl2);
    right_hand_pl2->setOrigin(0.5, 0.5);

    // pl1 score
    auto score_pl1_text = tgui::Label::create(); gui.add(score_pl1_text);
    string full_text_pl1_score = "(" + pl1_name + ") score: " + to_string(score_pl1_short);
    score_pl1_text->setText(full_text_pl1_score);

    score_pl1_text->getRenderer()->setTextColor(tgui::Color::White);
    score_pl1_text->getRenderer()->setFont(font);
    score_pl1_text->setPosition("1%", "2%");
    score_pl1_text->setTextSize(16);
    score_pl1_text->setOrigin(0, 0);

    // pl2 score
    auto score_pl2_text = tgui::Label::create("Score: 0"); gui.add(score_pl2_text);
    string full_text_pl2_score = "(" + pl2_name + ") score: " + to_string(score_pl2_short);
    score_pl2_text->setText(full_text_pl2_score);

    score_pl2_text->getRenderer()->setTextColor(tgui::Color::White); 
    score_pl2_text->getRenderer()->setFont(font);
    score_pl2_text->setPosition("99%", "2%");
    score_pl2_text->setTextSize(16);
    score_pl2_text->setOrigin(1, 0);

    // center button
    auto btn_tap = Button::create("Click me!"); gui.add(btn_tap);
    btn_tap->setRenderer(theme->getRenderer("gd_button"));
    btn_tap->setTextSize(28);
    btn_tap->setOrigin(0.5, 0.5);
    
    btn_tap->onPress([&]{
        
        play();
        
        if (number_cubes_pl1 > number_cubes_pl2) {
            score_pl1_short++;
            full_text_1 = "(" + pl1_name + ") score: " + to_string(score_pl1_short);
            score_pl1_text->setText(full_text_1);
        }
        if (number_cubes_pl1 < number_cubes_pl2) {
            score_pl2_short++;
            full_text_2 = "(" + pl2_name + ") score: " + to_string(score_pl2_short);
            score_pl2_text->setText(full_text_2);
        }
    });

    auto updateWidgetsLayout = [&]() {
        float currentWidth = gui.getView().getWidth();
        float currentHeight = gui.getView().getHeight();
        
        float scaleX = currentWidth / originalWidth;
        float scaleY = currentHeight / originalHeight;
        float scale = std::min(scaleX, scaleY);
        
        float offsetX = (currentWidth - (originalWidth * scale)) / 2.0f;
        float offsetY = (currentHeight - (originalHeight * scale)) / 2.0f;
        
        cup_pl1->setSize(positions[0].width * scale, positions[0].height * scale);
        cup_pl1->setPosition(offsetX + (positions[0].x * scale), offsetY + (positions[0].y * scale));
        
        cup_pl2->setSize(positions[1].width * scale, positions[1].height * scale);
        cup_pl2->setPosition(offsetX + (positions[1].x * scale), offsetY + (positions[1].y * scale));
        
        left_hand_pl1->setSize(positions[2].width * scale, positions[2].height * scale);
        left_hand_pl1->setPosition(offsetX + (positions[2].x * scale), offsetY + (positions[2].y * scale));
        
        right_hand_pl1->setSize(positions[3].width * scale, positions[3].height * scale);
        right_hand_pl1->setPosition(offsetX + (positions[3].x * scale), offsetY + (positions[3].y * scale));
        
        left_hand_pl2->setSize(positions[4].width * scale, positions[4].height * scale);
        left_hand_pl2->setPosition(offsetX + (positions[4].x * scale), offsetY + (positions[4].y * scale));
        
        right_hand_pl2->setSize(positions[5].width * scale, positions[5].height * scale);
        right_hand_pl2->setPosition(offsetX + (positions[5].x * scale), offsetY + (positions[5].y * scale));
        
        btn_tap->setSize(positions[6].width * scale, positions[6].height * scale);
        btn_tap->setPosition(offsetX + (positions[6].x * scale), offsetY + (positions[6].y * scale));
        
        unsigned int textSize = static_cast<unsigned int>(16 * scale);
        score_pl1_text->setTextSize(textSize);
        score_pl2_text->setTextSize(textSize);
        btn_tap->setTextSize(static_cast<unsigned int>(28 * scale));
    };

    updateWidgetsLayout();

    gui.onViewChange([&]{
        updateWidgetsLayout();
    });
    
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent()) {
            gui.handleEvent(*event);
            
            // quit - close window
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // render
        window.clear({62, 35, 0});
        gui.draw();
        window.display();
    }
}