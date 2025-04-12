#include "InputDevices.h"

// keyboard events

KeyboardEvent::KeyboardEvent()
	:
	type(EventType::Invalid),
	key(0u)
{
}

KeyboardEvent::KeyboardEvent(const EventType type, const unsigned char key)
	:
	type(type),
	key(key)
{
}

bool KeyboardEvent::IsPress() const
{
	return this->type == EventType::Press;
}

bool KeyboardEvent::IsRelease() const
{
	return this->type == EventType::Release;
}

bool KeyboardEvent::IsValid() const
{
	return this->type != EventType::Invalid;
}

unsigned char KeyboardEvent::GetKeyCode() const
{
	return this->key;
}


// mouse events 

MouseEvent::MouseEvent()
	:
	type(EventType::Invalid),
	x(0),
	y(0)
{}

MouseEvent::MouseEvent(EventType type, int x, int y)
	:
	type(type),
	x(x),
	y(y)
{}

bool MouseEvent::IsValid() const
{
	return this->type != EventType::Invalid;
}

MouseEvent::EventType MouseEvent::GetType() const
{
	return this->type;
}

MousePoint MouseEvent::GetPos() const
{
	return{ this->x,this->y };
}

int MouseEvent::GetPosX() const
{
	return this->x;
}

int MouseEvent::GetPosY() const
{
	return this->y;
}


// keyboard class

Keyboard::Keyboard()
{
	for (int i = 0; i < 256; i++)
		this->keyStates[i] = false; //Initialize all key states to off (false)
}

bool Keyboard::KeyIsPressed(const unsigned char keycode)
{
	return this->keyStates[keycode];
}

bool Keyboard::KeyBufferIsEmpty()
{
	return this->keyBuffer.empty();
}

bool Keyboard::CharBufferIsEmpty()
{
	return this->charBuffer.empty();
}

KeyboardEvent Keyboard::ReadKey()
{
	if (this->keyBuffer.empty()) //If no keys to be read?
	{
		return KeyboardEvent(); //return empty keyboard event
	}
	else
	{
		KeyboardEvent e = this->keyBuffer.front(); //Get first Keyboard Event from queue
		this->keyBuffer.pop(); //Remove first item from queue
		return e; //Returns keyboard event
	}
}

unsigned char Keyboard::ReadChar()
{
	if (this->charBuffer.empty()) //If no keys to be read?
	{
		return 0u; //return 0 (NULL char)
	}
	else
	{
		unsigned char e = this->charBuffer.front(); //Get first char from queue
		this->charBuffer.pop(); //Remove first char from queue
		return e; //Returns char
	}
}

void Keyboard::OnKeyPressed(const unsigned char key)
{
	this->keyStates[key] = true;
	this->keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Press, key));
}

void Keyboard::OnKeyReleased(const unsigned char key)
{
	this->keyStates[key] = false;
	this->keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Release, key));
}

void Keyboard::OnChar(const unsigned char key)
{
	this->charBuffer.push(key);
}

void Keyboard::EnableAutoRepeatKeys()
{
	this->autoRepeatKeys = true;
}

void Keyboard::DisableAutoRepeatKeys()
{
	this->autoRepeatKeys = false;
}

void Keyboard::EnableAutoRepeatChars()
{
	this->autoRepeatChars = true;
}

void Keyboard::DisableAutoRepeatChars()
{
	this->autoRepeatChars = false;
}

bool Keyboard::IsKeysAutoRepeat()
{
	return this->autoRepeatKeys;
}

bool Keyboard::IsCharsAutoRepeat()
{
	return this->autoRepeatChars;
}


// mouse class

void Mouse::OnLeftPressed(int x, int y)
{
	this->leftIsDown = true;
	MouseEvent me(MouseEvent::EventType::LPress, x, y);
	this->eventBuffer.push(me);
}

void Mouse::OnLeftReleased(int x, int y)
{
	this->leftIsDown = false;
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::LRelease, x, y));
}

void Mouse::OnRightPressed(int x, int y)
{
	this->rightIsDown = true;
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::RPress, x, y));
}

void Mouse::OnRightReleased(int x, int y)
{
	this->rightIsDown = false;
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::RRelease, x, y));
}

void Mouse::OnMiddlePressed(int x, int y)
{
	this->mbuttonDown = true;
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::MPress, x, y));
}

void Mouse::OnMiddleReleased(int x, int y)
{
	this->mbuttonDown = false;
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::MRelease, x, y));
}

void Mouse::OnWheelUp(int x, int y)
{
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::WheelUp, x, y));
}

void Mouse::OnWheelDown(int x, int y)
{
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::WheelDown, x, y));
}

void Mouse::OnMouseMove(int x, int y)
{
	this->x = x;
	this->y = y;
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::Move, x, y));
}

void Mouse::OnMouseMoveRaw(int x, int y)
{
	this->eventBuffer.push(MouseEvent(MouseEvent::EventType::RAW_MOVE, x, y));
}

bool Mouse::IsLeftDown()
{
	return this->leftIsDown;
}

bool Mouse::IsMiddleDown()
{
	return this->mbuttonDown;
}

bool Mouse::IsRightDown()
{
	return this->rightIsDown;
}

int Mouse::GetPosX()
{
	return this->x;
}

int Mouse::GetPosY()
{
	return this->y;
}

MousePoint Mouse::GetPos()
{
	return{ this->x, this->y };
}

bool Mouse::EventBufferIsEmpty()
{
	return this->eventBuffer.empty();
}

MouseEvent Mouse::ReadEvent()
{
	if (this->eventBuffer.empty())
	{
		return MouseEvent();
	}
	else
	{
		MouseEvent e = this->eventBuffer.front(); //Get first event from buffer
		this->eventBuffer.pop(); //Remove first event from buffer
		return e;
	}
}

