#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <ctime>
#include <thread>
#include <string>
#include <sstream>
#include <chrono>
#include <iostream>
#include <Windows.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int FIELD_WIDTH = 12;
const int FIELD_HEIGHT = 18;

int field[FIELD_WIDTH * FIELD_HEIGHT];

int tetrominos[7][16] =
{
	{ 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0 },

	{ 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0 },

	{ 0, 3, 0, 0, 0, 3, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0 },

	{ 0, 0, 0, 0, 0, 4, 4, 0, 0, 4, 4, 0, 0, 0, 0, 0 },

	{ 0, 0, 5, 0, 0, 5, 5, 0, 0, 0, 5, 0, 0, 0, 0, 0 },

	{ 0, 6, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0 },

	{ 0, 7, 7, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0 }
};

struct Point
{
	int x = 0;
	int y = 0;
};

std::string IntToString(int number)
{
	std::ostringstream os;
	os << number;
	return os.str();
}

void CheckColor(sf::Sprite& s, int tetromino)
{
	switch (tetromino)
	{
	case 0: s.setColor(sf::Color(255.0f, 0.0f, 0.0f, 255.0f)); break;
	case 1: s.setColor(sf::Color(0.0f, 255.0f, 0.0f, 255.0f)); break;
	case 2: s.setColor(sf::Color(0.0f, 0.0f, 255.0f, 255.0f)); break;
	case 3: s.setColor(sf::Color(127.5f, 127.5f, 127.5f, 255.0f)); break;
	case 4: s.setColor(sf::Color(255.0f, 255.0f, 0.0f, 255.0f)); break;
	case 5: s.setColor(sf::Color(255.0f, 0.0f, 255.0f, 255.0f)); break;
	case 6: s.setColor(sf::Color(0.0f, 255.0f, 255.0f, 255.0f)); break;
	}
}


// The function is taken from:
// https://www.youtube.com/watch?v=8OK8_tHeCIA
int Rotate(int px, int py, int r)
{
	int pi = 0;
	switch (r % 4)
	{
	case 0: // 0 degrees			// 0  1  2  3
		pi = py * 4 + px;			// 4  5  6  7
		break;						// 8  9 10 11
									//12 13 14 15

	case 1: // 90 degrees			//12  8  4  0
		pi = 12 + py - (px * 4);	//13  9  5  1
		break;						//14 10  6  2
									//15 11  7  3

	case 2: // 180 degrees			//15 14 13 12
		pi = 15 - (py * 4) - px;	//11 10  9  8
		break;						// 7  6  5  4
									// 3  2  1  0

	case 3: // 270 degrees			// 3  7 11 15
		pi = 3 - py + (px * 4);		// 2  6 10 14
		break;						// 1  5  9 13
	}								// 0  4  8 12

	return pi;
}

// Also taken from:
// https://www.youtube.com/watch?v=8OK8_tHeCIA
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
 	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			int pi = Rotate(px, py, nRotation);
	
			int fi = (nPosY + py) * FIELD_WIDTH + (nPosX + px);
	
			if (nPosX + px >= 0 && nPosX + px < FIELD_WIDTH)
			{
				if (nPosY + py >= 0 && nPosY + py < FIELD_HEIGHT)
				{
					if (tetrominos[nTetromino][pi] != 0 && field[fi] != 0)
							return false;
				}
			}
		}
	return true;
}

bool isRowFull(int row)
{
	for (int x = 1; x <= FIELD_WIDTH - 2; x++)
	{
		if (field[row * FIELD_WIDTH + x] == 0)
			return false;
	}
	return true;
}

int main()
{
	srand(time(0));
	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Tetris");

	sf::Font f1;
	// Font taken from:
	// https://www.dafont.com/brady-bunch.font
	f1.loadFromFile(("font/BradBunR.ttf"));

	sf::Text gameOverText, commandText, scoreText, pauseText, hintText, hintText2;

	pauseText.setFont(f1);
	pauseText.setString("Game Paused!");
	pauseText.setCharacterSize(50);
	gameOverText.setFillColor(sf::Color::White);

	gameOverText.setFont(f1);
	gameOverText.setString("Game Over!");
	gameOverText.setCharacterSize(50);
	gameOverText.setFillColor(sf::Color::White);

	commandText.setFont(f1);
	commandText.setCharacterSize(25);
	commandText.setFillColor(sf::Color::White);

	scoreText.setFont(f1);
	scoreText.setCharacterSize(45);
	scoreText.setFillColor(sf::Color::Blue);

	hintText.setFont(f1);
	hintText.setString("Space: Pause game");
	hintText.setCharacterSize(25);
	hintText.setFillColor(sf::Color::Blue);

	hintText2.setFont(f1);
	hintText2.setString("ESC: Exit game");
	hintText2.setCharacterSize(30);
	hintText2.setFillColor(sf::Color::Blue);


	// Block and Background image taken from:
	// https://learnopengl.com/In-Practice/2D-Game/Levels
	sf::Texture t1, t2;
	t1.loadFromFile("images/block.png");
	t2.loadFromFile("images/background.jpg");

	sf::Sprite s(t1);
	s.setScale(sf::Vector2f(0.25f, 0.25f));
	float sprSize = t1.getSize().x * 0.25f; //Only need 1 axis for size since sprite is a square

	sf::IntRect bndSize(0, 0, 320, 544);
	sf::Sprite background(t2, bndSize);
	background.setPosition(SCREEN_WIDTH / 2.0f - bndSize.width / 2.0f, 32.0f);

	int tetromino = rand() % 7;

	Point currentPos = 
	{
		FIELD_WIDTH / 2 - 2,
		0
	};

	char rotation = 0;

	int speed = 20;
	int speedCount = 0;
	bool canGoDown;

	int score = 0;

	Point PiecePos[4];
	int p = 0;
	
	bool GameOver = false;
	bool isPaused = false;

	for (int x = 0; x < FIELD_WIDTH; x++)
		for (int y = 0; y < FIELD_HEIGHT; y++)
			field[y * FIELD_WIDTH + x] = ((x == 0 || x == FIELD_WIDTH - 1) || (y == FIELD_HEIGHT - 1)) ? 8 : 0;
	
	while (window.isOpen())
	{
		if (!GameOver && !isPaused)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			speedCount++;
			canGoDown = (speed == speedCount);

			if (canGoDown)
			{
				speedCount = 0;
				if (DoesPieceFit(tetromino, rotation, currentPos.x, currentPos.y + 1))
					currentPos.y++;
				else
				{
					for (int i = 0; i < 4; i++)
					{
						//offset by 1 since 1st tetromino's id is 0 which field will take as empty cell
						field[PiecePos[i].y * FIELD_WIDTH + PiecePos[i].x] = tetromino + 1;
					}

					for (int y = 0; y < FIELD_HEIGHT - 1; y++)
					{
						if (isRowFull(y))
						{
							for (int i = y; i >= 0; i--)
								for (int ј = 1; ј < FIELD_WIDTH - 1; ј++)
									field[i * FIELD_WIDTH + ј] = field[(i - 1) * FIELD_WIDTH + ј];
							score += 10;
						}
					}

					tetromino = rand() % 7;
					currentPos = {
						FIELD_WIDTH / 2 - 2,
						0
					};
					rotation = 0;

					GameOver = !DoesPieceFit(tetromino, rotation, currentPos.x, currentPos.y);
				}
			}

		}
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					event.type = sf::Event::Closed;
				if (!GameOver && !isPaused) 
				{
					currentPos.x -= (event.key.code == sf::Keyboard::Left &&
						DoesPieceFit(tetromino, rotation, currentPos.x - 1, currentPos.y)) ? 1 : 0;
					currentPos.x += (event.key.code == sf::Keyboard::Right &&
						DoesPieceFit(tetromino, rotation, currentPos.x + 1, currentPos.y)) ? 1 : 0;
					currentPos.y += (event.key.code == sf::Keyboard::Down &&
						DoesPieceFit(tetromino, rotation, currentPos.x, currentPos.y + 1)) ? 1 : 0;
					rotation += (event.key.code == sf::Keyboard::Up &&
						DoesPieceFit(tetromino, rotation + 1, currentPos.x, currentPos.y)) ? 1 : 0;
				}
				if (event.key.code == sf::Keyboard::Space)
					if (GameOver)
					{
						for (int x = 0; x < FIELD_WIDTH; x++)
							for (int y = 0; y < FIELD_HEIGHT; y++)
								field[y * FIELD_WIDTH + x] = ((x == 0 || x == FIELD_WIDTH - 1) || (y == FIELD_HEIGHT - 1)) ? 8 : 0;
						tetromino = rand() % 7;
						currentPos = {
							FIELD_WIDTH / 2 - 2,
							0
						};
						rotation = 0;
						score = 0;
						GameOver = false;
					}
					else
						isPaused = !isPaused;
			}
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear(sf::Color(0.0f, 200.0f, 255.0f, 255.0f));
		window.draw(background);
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				int piecePos = tetrominos[tetromino][Rotate(i, j, rotation)];
				if (piecePos != 0)
				{
					CheckColor(s, tetromino);

					s.setPosition(background.getPosition().x + (currentPos.x + i) * sprSize - sprSize,
						background.getPosition().y + (currentPos.y + j) * sprSize);

					//Get field postition indexes for locking
					PiecePos[p].x = (s.getPosition().x - background.getPosition().x) / sprSize + 1;
					PiecePos[p].y = (s.getPosition().y - background.getPosition().y) / sprSize;
					p++;

					window.draw(s);
				}
			}
		}
		p = 0;

		for (int y = 0; y < FIELD_HEIGHT; y++)
		{
			for (int x = 0; x < FIELD_WIDTH; x++)
			{
				if (field[y * FIELD_WIDTH + x] != 0 && field[y * FIELD_WIDTH + x] != 8)
				{
					//offset by 1 because of 1st tetromino's id is 0 which field will take as empty cell
					CheckColor(s, field[y * FIELD_WIDTH + x] - 1);
					s.setPosition(x * sprSize, y * sprSize);
					s.move(background.getPosition().x - sprSize, sprSize);
					window.draw(s);
				}
				//std::cout << field[y * FIELD_WIDTH + x];
			}
			//std::cout << "\n";
		}
		//std::cout << "\n";

		if (isPaused)
		{
			pauseText.setPosition(SCREEN_WIDTH / 2.0f - pauseText.getCharacterSize() * 2.5f, SCREEN_HEIGHT / 2.0f - gameOverText.getCharacterSize() * 1.5f);
			commandText.setString("Press Space to continue");
			commandText.setPosition(SCREEN_WIDTH / 2.0f - 125, SCREEN_HEIGHT / 2.0f - gameOverText.getCharacterSize() / 2.0f);
			window.draw(pauseText);
			window.draw(commandText);
		}

		if (GameOver)
		{
			gameOverText.setPosition(SCREEN_WIDTH / 2.0f - gameOverText.getCharacterSize() * 2, SCREEN_HEIGHT / 2.0f - gameOverText.getCharacterSize() * 1.5f);
			commandText.setString("Press Space to start again");
			commandText.setPosition(SCREEN_WIDTH / 2.0f - 125, SCREEN_HEIGHT / 2.0f - gameOverText.getCharacterSize() / 2.0f);
			window.draw(gameOverText);
			window.draw(commandText);
		}


		hintText.setPosition(10, SCREEN_HEIGHT - 32);
		hintText2.setPosition(SCREEN_WIDTH - 180, SCREEN_HEIGHT - 38);

		window.draw(hintText);
		window.draw(hintText2);

		std::string number = IntToString(score);
		scoreText.setString("Score: " + number);
		scoreText.setPosition(10, 0);
		window.draw(scoreText);

		window.display();
	}
	return 0;
}