#pragma once

#include <fstream>
#include <memory>

#include "ZClient.hpp"
#include "weather_mapping.hpp"

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPixmap>
#include <QByteArray>
#include <QBoxLayout>

class FetcherWindow : public QWidget
{
	Q_OBJECT

public:
	FetcherWindow();
	void Connect(const std::string& host);
	void InitializeWeatherMapper();

private:
	const QString _defaultDataOutput;
	std::unique_ptr<WeatherMapper> _weatherMapper;
	std::unique_ptr<ZClient> _client;
	std::string _lastWeatherType;
	std::atomic<bool> _connected;

	void _setupLayout(QLineEdit* cityInput, QLabel* fetchedDataLabel, QLabel* weatherPictureLabel);
	std::string _fetchDataAndUpdateDb(const std::string& city);
	std::string _getWeatherData(const std::string& city);
	void _applyStyleSheet();
	void Reconnect();
	
private slots:
	void _showSpecificPicture(const std::string& weatherType);
	void _displayWeatherData(const std::string& weatherData);

signals:
	void ConnectionFailed();
};