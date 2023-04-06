#include "Car.h"
#include <exception>
Car::Car(const char* imagePath)
    : _brain{ {11, 5}, relu_activate }, _eye{ 18, 11} {
    sf::Image image;
    if (!image.loadFromFile(imagePath))
        throw std::invalid_argument("Can't load car image.");
    image.createMaskFromColor(maskColor);
    if (!_texture.loadFromImage(image))
        throw std::runtime_error("Can't create texture from car image.");
    _sprite.setTexture(_texture);
    auto bound = _sprite.getLocalBounds();
    _sprite.setOrigin(bound.width / 2, bound.height / 2);
    _sprite.setScale(scale);
    _sprite.setRotation(-90);
    bound = _sprite.getGlobalBounds();
    _localEyePosition.x = 0;
    _localEyePosition.y = 0;

    _eye.setPosition(Helper::addVector(_localEyePosition, _sprite.getPosition()));
    lap = 0;
    lastCheckPoint = 1;
    forward.x = 1;
    forward.y = 0;
}

Car::Car(const Car& p1, const Car& p2)
    : _brain{p1._brain.reproduce(p2._brain)}, _eye{p1._eye}, _texture{p1._texture}, _sprite{p1._sprite} {
    
    auto bound = _sprite.getLocalBounds();
    _sprite.setOrigin(bound.width / 2, bound.height / 2);
    _sprite.setScale(scale);
    _sprite.setRotation(-90);
    bound = _sprite.getGlobalBounds();
    _localEyePosition.x = 0;
    _localEyePosition.y = 0;

    _eye.setPosition(Helper::addVector(_localEyePosition, _sprite.getPosition()));
    lap = 0;
    lastCheckPoint = 1;
    forward.x = 1;
    forward.y = 0;
}

Car::Car(const Car& base)
    : _brain{base._brain}, _eye{base._eye}, _texture{base._texture}, _sprite{base._sprite} {
    
    auto bound = _sprite.getLocalBounds();
    _sprite.setOrigin(bound.width / 2, bound.height / 2);
    _sprite.setScale(scale);
    _sprite.setRotation(-90);
    bound = _sprite.getGlobalBounds();
    _localEyePosition.x = 0;
    _localEyePosition.y = 0;

    _eye.setPosition(Helper::addVector(_localEyePosition, _sprite.getPosition()));
    lap = 0;
    lastCheckPoint = 1;
    forward.x = 1;
    forward.y = 0;
}



Car::Car(): Car("resources/Car.jpg") {}



void Car::translate(const sf::Vector2f& velocity) {
    _sprite.move(velocity);
    _eye.setPosition(Helper::addVector(_sprite.getPosition(), _localEyePosition));
}

void Car::think(const Path& path) {
    Eigen::VectorXf input = _eye.senseDistance(path);
    Eigen::VectorXf output = _brain.forward_propagate(input);
    unsigned decision = std::max_element(output.begin(), output.end()) - output.begin();
    if (decision == 0)
        movement.rotate = Rotate::Up;
    else if (decision == 1)
        movement.rotate = Rotate::Down;
    else if (decision == 2)
        movement.moveforward = true;
    else if (decision == 3)
        movement.moveforward = false;
    else if (decision == 4)
        movement.rotate = Rotate::None;
    
    
}

// std::pair<sf::Vector2f, int> Car::FindPositionOnPath(const Path& path) const {
//     const sf::VertexArray& vArray = path.getSplineArray();
//     int n = vArray.getVertexCount();
//     sf::Vector2f pos = _sprite.getPosition();
//     double minDistance = 9999999;
//     std::optional<sf::Vector2f> projectionPoint{};
//     int index = -1;
//     for (int i = 1; i < n - 1; i++) {
//         std::optional<sf::Vector2f> temp = Helper::pointProjectToLine(pos, vArray[i].position, vArray[i + 1].position);
//         if (temp.has_value()) {
//             double tempDistance = Helper::distance(pos, temp.value());
//             if (tempDistance <= Config::path_width && tempDistance < minDistance) {
//                 projectionPoint = temp;
//                 minDistance = tempDistance;
//                 index = i;
//             }
//         }
//     }
//     if (!projectionPoint.has_value()) {
//         for (int i = 1; i < n - 1; i++) {
//             double tempDistance = Helper::distance(vArray[i].position, pos);
//             if (tempDistance < minDistance) {
//                 minDistance = tempDistance;
//                 projectionPoint = vArray[i].position;
//                 index = i;
//             }
//         }
//     }
//     return std::make_pair(projectionPoint.value(), index);
// }

double Car::getTravelDisance(const Path& path) const {
    double current_lap_distance = path.getDistanceTravel(_sprite.getPosition());
    
    return current_lap_distance + lap * path.getPathLength();
}



void Car::update(const Path& path) {
    auto currentPosition = path.getPositionOnSpline(_sprite.getPosition());
    int n = path.getSplineArray().getVertexCount();
    if (lastCheckPoint >= 1 && lastCheckPoint <= 3 && currentPosition.second <= n - 2 && currentPosition.second >= n - 4) {
        lap--;
    }
    else if (lastCheckPoint <= n - 2 && lastCheckPoint >= n - 4 && currentPosition.second >= 1 && currentPosition.second <= 3) {
        lap++;
    }
    lastCheckPoint = currentPosition.second;
}

void Car::showLine(sf::RenderTarget& target, const Path& path) const {
    auto positionOnPath = path.getPositionOnSpline(_sprite.getPosition());
    Helper::drawLine(target, positionOnPath.first, _sprite.getPosition(), sf::Color::Red);
}

void Car::move() {
    if (movement.moveforward) {
        moveForward();
        accelerator += 0.3f;
    }
    else
        accelerator = 0.0f;
    if (movement.moveforward && movement.rotate == Rotate::Up) {
        rotate(rotateAngle);
        accelerator = 0.0f;
    }
    else if (movement.moveforward && movement.rotate == Rotate::Down) {
        rotate(-rotateAngle);
        accelerator = 0.0f;
    }

}

void Car::saveBrainToFile(const char* fName) const {
    std::ofstream fout{fName};
    if (!fout.is_open())
        throw std::runtime_error("Can't open file to save brain.");
    fout << _brain;
    fout.close();
}

void Car::readBrainFromFile(const char* fName) {
    std::ifstream fin{fName};
    if (!fin.is_open())
        throw std::runtime_error("Can't open file to read brain.");
    fin >> _brain;
    fin.close();
}

void Car::mutate() {
    _brain.changeRandom();
}


void Car::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(_sprite);
}

void Car::showEyeLine(sf::RenderTarget& target, const Path& path) {
    _eye.draw(target, path);
}

void Car::rotate(float angle) {
    _sprite.rotate(-angle);
    Helper::rotate(_localEyePosition, angle);
    _eye.setPosition(Helper::addVector(_localEyePosition, _sprite.getPosition()));
    Helper::rotate(forward, angle);
    _eye.rotate(angle);
}
