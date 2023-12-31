#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include "imgui-SFML.h"
#include "imgui.h"

float RAnd(float w1, float w2) { return w1 + w2 + std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }

float ROr(float w1, float w2) { return w1 + w2 - std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }

sf::Color interpolateColors(const sf::Color &colorFirst, const sf::Color &colorSec, float k)
{
	sf::Color val;
	val.r = static_cast<sf::Uint8>(colorFirst.r + (colorSec.r - colorFirst.r) * k);
	val.g = static_cast<sf::Uint8>(colorFirst.g + (colorSec.g - colorFirst.g) * k);
	val.b = static_cast<sf::Uint8>(colorFirst.b + (colorSec.b - colorFirst.b) * k);
	val.a = static_cast<sf::Uint8>(colorFirst.a + (colorSec.a - colorFirst.a) * k);

	return val;
}

class RFuncSprites : public sf::Sprite
{
public:
	void create(const sf::Vector2u &size, const int getElem)
	{
		_image.create(size.x, size.y, sf::Color::Cyan);
		_texture.loadFromImage(_image);
		setTexture(_texture);

		_firstColor = sf::Color::Black;
		_secondColor = sf::Color::White;

		_getNorm = getElem;
	}

	void DrawRFunc(const std::function<float(const sf::Vector2f &)> &rfunc, const sf::FloatRect &subSpace)
	{
		sf::Vector2f spaceStep = {subSpace.width / static_cast<float>(_image.getSize().x),
								  subSpace.height / static_cast<float>(_image.getSize().y)};

		for (int x = 0; x < _image.getSize().x - 1; ++x)
		{
			for (int y = 0; y < _image.getSize().y - 1; ++y)
			{
				sf::Vector2f spacePoint1 = {subSpace.left + static_cast<float>(x) * spaceStep.x,
											subSpace.top + static_cast<float>(y) * spaceStep.y};

				const float z1 = rfunc(spacePoint1);

				sf::Vector2f spacePoint2 = {subSpace.left + static_cast<float>(x + 1) * spaceStep.x,
											subSpace.top + static_cast<float>(y) * spaceStep.y};

				const float z2 = rfunc(spacePoint2);

				sf::Vector2f spacePoint3 = {subSpace.left + static_cast<float>(x) * spaceStep.x,
											subSpace.top + static_cast<float>(y + 1) * spaceStep.y};

				const float z3 = rfunc(spacePoint3);

				const float A = calculateDeterminant3x3({
					{spacePoint1.y, z1, 1},
					{spacePoint2.y, z2, 1},
					{spacePoint3.y, z3, 1},
				});

				const float B = calculateDeterminant3x3({
					{spacePoint1.x, z1, 1},
					{spacePoint2.x, z2, 1},
					{spacePoint3.x, z3, 1},
				});

				const float C = calculateDeterminant3x3({
					{spacePoint1.x, spacePoint1.y, 1},
					{spacePoint2.x, spacePoint2.y, 1},
					{spacePoint3.x, spacePoint3.y, 1},
				});

				const float D = calculateDeterminant3x3({
					{spacePoint1.x, spacePoint1.y, z1},
					{spacePoint2.x, spacePoint2.y, z2},
					{spacePoint3.x, spacePoint3.y, z3},
				});

				const float det = std::sqrt(A * A + B * B + C * C + D * D);

				float nx = A / det;
				float ny = B / det;
				float nz = C / det;
				float nw = D / det;

				float getNorm = nx;

				switch (_getNorm)
				{
				case 0:
					break;
				case 1:
					getNorm = ny;
					break;
				case 2:
					getNorm = nz;
					break;
				case 3:
					getNorm = nw;
					break;
				}

				auto pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + getNorm) / 2);
				_image.setPixel(x, y, pixelColor);
			}
		}

		_texture.loadFromImage(_image);
	}

	float calculateDeterminant3x3(const std::vector<std::vector<float>> &matrix)
	{
		if (matrix.size() != 3 || matrix[0].size() != 3 || matrix[1].size() != 3 || matrix[2].size() != 3)
		{
			throw std::runtime_error("Wrong");
		}

		return matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
			matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
			matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
	}

	void upd(const sf::Color &firstColor, const sf::Color &secondColor)
	{
		for (int x = 0; x < _image.getSize().x - 1; ++x)
		{
			for (int y = 0; y < _image.getSize().y - 1; ++y)
			{
				float t =
					(static_cast<float>(_image.getPixel(x, y).r) - _firstColor.r) / (_secondColor.r - _firstColor.r);
				auto pixelColor = interpolateColors(firstColor, secondColor, t);
				_image.setPixel(x, y, pixelColor);
			}
		}

		_firstColor = firstColor;
		_secondColor = secondColor;
		_texture.loadFromImage(_image);
	}

	void saveImg(const std::string &filename) { _image.saveToFile(filename); }

private:
	sf::Color _firstColor;
	sf::Color _secondColor;
	sf::Texture _texture;
	sf::Image _image;
	int _getNorm;
};



int main()
{
	sf::RenderWindow window(sf::VideoMode(400, 400), "Lab2");
	window.setFramerateLimit(60);
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "ImGui initialization failed\n";
		return -1;
	}

	auto spriteSize = sf::Vector2u{window.getSize().x / 2, window.getSize().y / 2};

	RFuncSprites RFuncSpritesNx;
	RFuncSpritesNx.create(spriteSize, 0);

	RFuncSprites RFuncSpritesNy;
	RFuncSpritesNy.create(spriteSize, 1);
	RFuncSpritesNy.setPosition(spriteSize.x, 0);

	RFuncSprites RFuncSpritesNz;
	RFuncSpritesNz.create(spriteSize, 2);
	RFuncSpritesNz.setPosition(0, spriteSize.y);

	RFuncSprites RFuncSpritesNw;
	RFuncSpritesNw.create(spriteSize, 3);
	RFuncSpritesNw.setPosition(spriteSize.x, spriteSize.y);

	std::function<float(const sf::Vector2f &)> rFunctions[6];

	rFunctions[0] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) - std::cos(point.y); };
	rFunctions[1] = [](const sf::Vector2f &point) -> float { return point.x * point.y - 10; };
	rFunctions[2] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) * std::cos(point.y); };
	rFunctions[3] = [](const sf::Vector2f &point) -> float { return std::cos(point.x + point.y); };
	rFunctions[4] = [](const sf::Vector2f &point) -> float { return point.x * point.x + point.y * point.y - 500; };
	rFunctions[5] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) / std::cos(point.y)-40; };
	

	std::function<float(const sf::Vector2f &)> complexFunction = [&rFunctions](const sf::Vector2f &point) -> float
	{
		return RAnd(RAnd(
			RAnd(ROr(RAnd(rFunctions[0](point), rFunctions[1](point)), rFunctions[2](point)),
					   rFunctions[3](point)),
			rFunctions[4](point)), rFunctions[5](point));
	};

	sf::FloatRect subSpace(-10.f, -10.f, 20.f, 20.f);



	sf::Clock deltaClock;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Menu");

		ImVec4 firstColor = ImVec4(0, 0, 0, 1);
		ImVec4 secondColor = ImVec4(1, 1, 1, 1);


		auto sfFirstColor = sf::Color(static_cast<sf::Uint8>(firstColor.x * 255), static_cast<sf::Uint8>(firstColor.y * 255),
						  static_cast<sf::Uint8>(firstColor.z * 255), static_cast<sf::Uint8>(firstColor.w * 255));

		auto sfSecondColor =
				sf::Color(static_cast<sf::Uint8>(secondColor.x * 255), static_cast<sf::Uint8>(secondColor.y * 255),
						  static_cast<sf::Uint8>(secondColor.z * 255), static_cast<sf::Uint8>(secondColor.w * 255));

		RFuncSpritesNx.DrawRFunc(complexFunction, subSpace);
		RFuncSpritesNx.upd(sfFirstColor, sfSecondColor);

		RFuncSpritesNy.DrawRFunc(complexFunction, subSpace);
		RFuncSpritesNy.upd(sfFirstColor, sfSecondColor);

		RFuncSpritesNz.DrawRFunc(complexFunction, subSpace);
		RFuncSpritesNz.upd(sfFirstColor, sfSecondColor);

		RFuncSpritesNw.DrawRFunc(complexFunction, subSpace);
		RFuncSpritesNw.upd(sfFirstColor, sfSecondColor);
		

		if (ImGui::Button("Save Image"))
		{
			RFuncSpritesNx.saveImg("nx.png");
			RFuncSpritesNy.saveImg("ny.png");
			RFuncSpritesNz.saveImg("nz.png");
			RFuncSpritesNw.saveImg("nw.png");
		}

		ImGui::End();

		window.clear();

		window.draw(RFuncSpritesNx);
		window.draw(RFuncSpritesNy);
		window.draw(RFuncSpritesNz);
		window.draw(RFuncSpritesNw);

		ImGui::SFML::Render(window);

		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}
