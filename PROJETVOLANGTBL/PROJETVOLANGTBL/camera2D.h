#pragma once
#include <QVector2D>
class Camera2D
{
	public :
		Camera2D(QVector2D startPos, float zoom = 1.0f);
		~Camera2D();

		void setPosition(QVector2D pos);
		void setZoom(float zoom);
		QVector2D getPosition() const;
		float getZoom() const;

	private:
		QVector2D pos;
		float zoom;

};

