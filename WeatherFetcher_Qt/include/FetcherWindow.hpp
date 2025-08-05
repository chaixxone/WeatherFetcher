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
	void Connect();
	void InitializeWeatherMapper();
	void SetHost(const std::string& host) noexcept;

private:
	const QString _defaultDataOutput;
	std::unique_ptr<WeatherMapper> _weatherMapper;
	std::unique_ptr<ZClient> _client;
	std::string _lastWeatherType;
	std::string _host;
	std::atomic<bool> _connected;

	void SetupLayout(QLineEdit* cityInput, QLabel* fetchedDataLabel, QLabel* weatherPictureLabel);
	std::string FetchDataAndUpdateDb(const std::string& city);
	std::string GetWeatherData(const std::string& city);
	void ApplyStyleSheet();
	void Reconnect();
	
private slots:
	void ShowSpecificPicture(const std::string& weatherType);
	void DisplayWeatherData(const std::string& weatherData);

signals:
	void ConnectionFailed();
};