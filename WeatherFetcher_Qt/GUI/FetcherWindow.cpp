#include "FetcherWindow.hpp"

#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QRegularExpression>

FetcherWindow::FetcherWindow() : _defaultDataOutput("No data fetched..."), _client(new ZClient), _lastWeatherType(""), _connected(false)
{
	auto cityInput = new QLineEdit(this);
	auto fetchedDataLabel = new QLabel(this);
	auto weatherPictureLabel = new QLabel(this);

	cityInput->setObjectName("CityInput");
	cityInput->setPlaceholderText("Enter city");
	cityInput->setFixedSize(200, 40);

	fetchedDataLabel->setText(_defaultDataOutput);
	fetchedDataLabel->setObjectName("WeatherData");
	fetchedDataLabel->setWordWrap(true);
	fetchedDataLabel->setFixedWidth(256);
	fetchedDataLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		
	weatherPictureLabel->setObjectName("WeatherPicture");
	weatherPictureLabel->hide();
	weatherPictureLabel->setFixedSize(256, 256);

	connect(cityInput, &QLineEdit::returnPressed, this, [this, cityInput, fetchedDataLabel, weatherPictureLabel]() {
		QString cityText = cityInput->text().trimmed();

		if (cityText.isEmpty())
		{
			return;
		}

		std::string cityTextStd = cityText.toStdString();		
		_displayWeatherData(cityTextStd);
		auto fetchedDataStr = fetchedDataLabel->text();

		if (fetchedDataStr != _defaultDataOutput)
		{
			QRegularExpression regex("(?<=\\n)(.*?)(?=\\n)");
			QRegularExpressionMatch match = regex.match(fetchedDataStr);
			std::string currentWeather = match.captured(1).toStdString();

			if (_lastWeatherType != currentWeather || _lastWeatherType.empty())
			{
				_showSpecificPicture(currentWeather);
				_lastWeatherType = currentWeather;
			}
		}
	});

	connect(this, &FetcherWindow::ConnectionFailed, this, [this]() {
		auto connectionFailureMessage = new QMessageBox(this);
		connectionFailureMessage->setText("Couldn't connect to API\nWould you like to reconnect?");
		connectionFailureMessage->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		connectionFailureMessage->show();

		int buttonCode = connectionFailureMessage->exec();

		if (buttonCode == QMessageBox::StandardButton::Yes)
		{
			Reconnect();
		}
		else
		{
			close();
		}
	});

	setFixedSize(800, 600);
	_setupLayout(cityInput, fetchedDataLabel, weatherPictureLabel);
	_applyStyleSheet();

	QIcon icon(":/WetherFetcher_Qt/logo.png");
	setWindowIcon(icon);
}

void FetcherWindow::SetHost(const std::string& host) noexcept
{
	_host = host;
}

void FetcherWindow::Connect()
{
	std::thread connectionJob([this]() {
		try
		{
			_client->Connect(_host);
			_connected.store(true, std::memory_order_relaxed);
			InitializeWeatherMapper();
		}
		catch (const std::runtime_error& e)
		{
			// TODO: inform user with message box about connection failure
			std::cerr << e.what() << '\n';
		}
	});

	const int timeoutDuration = 5000;

	QTimer::singleShot(std::chrono::milliseconds(timeoutDuration), [&]() {
		if (!_connected.load())
		{
			_client->DestroyClientWork();
		}
	});

	connectionJob.detach();
}

void FetcherWindow::InitializeWeatherMapper()
{
	try
	{		
		std::string key = _client->MakeRequest("get_api_key", "");
		_weatherMapper = std::make_unique<WeatherMapper>(key);
	}
	catch (const std::runtime_error& e)
	{
		// TODO handle exception
	}
}

void FetcherWindow::Reconnect()
{
	_client.reset(new ZClient());
	Connect();
}

void FetcherWindow::_applyStyleSheet()
{
	QFile file(":/WetherFetcher_Qt/style.qss");

	if (!file.open(QFile::ReadOnly | QFile::Text)) 
	{
		qWarning("Cannot open file %s for reading: %s", qPrintable(file.fileName()), qPrintable(file.errorString()));
		return;
	}

	QTextStream ts(&file);
	QString styleSheet = ts.readAll();
	setStyleSheet(styleSheet);
}

void FetcherWindow::_setupLayout(QLineEdit* cityInput, QLabel* fetchedDataLabel, QLabel* weatherPictureLabel)
{
	fetchedDataLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	auto dataAndPictureLayout = new QHBoxLayout;
	dataAndPictureLayout->addWidget(fetchedDataLabel);
	dataAndPictureLayout->addWidget(weatherPictureLabel);
	dataAndPictureLayout->setAlignment(Qt::AlignVCenter);

	auto mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(cityInput);
	mainLayout->addLayout(dataAndPictureLayout);
	mainLayout->setAlignment(Qt::AlignHCenter);
	mainLayout->setContentsMargins(0, height() * 0.25, 0, 0);

	setLayout(mainLayout);
}

void FetcherWindow::_showSpecificPicture(const std::string& weatherType)
{
	auto data = _client->MakeRequest("get_image", weatherType);
	QByteArray byteArray(data.c_str(), data.size());
	
	static auto weatherPictureLabel = findChild<QLabel*>("WeatherPicture");

	QPixmap pixmap;
	pixmap.loadFromData(byteArray);

	if (pixmap.width() >= 300)
	{
		pixmap = std::move(pixmap.scaled(pixmap.width() / 2, pixmap.height() / 2));
	}

	weatherPictureLabel->setPixmap(pixmap);
	weatherPictureLabel->show();
}

static bool hasRecentData(const QString& databaseSnapshotDateTime)
{
	QString cleanedDateTimeStr = databaseSnapshotDateTime;
	cleanedDateTimeStr.remove('"');

	QDateTime dbDateTime = QDateTime::fromString(cleanedDateTimeStr, "yyyy-MM-dd HH:mm:ss");

	QDateTime currentDateTime = QDateTime::currentDateTime();

	qint64 secondsDifference = dbDateTime.secsTo(currentDateTime);
	const qint64 oneHourInSeconds = 3600;

	return secondsDifference <= oneHourInSeconds;
}

std::string FetcherWindow::_fetchDataAndUpdateDb(const std::string& city)
{
	auto snapshot = _weatherMapper->FetchWeatherData(city);
	_client->MakeRequest("insert_weather_data", snapshot);
	return snapshot;
}

std::string FetcherWindow::_getWeatherData(const std::string& city)
{
	std::string snapshot = _client->MakeRequest("get_weather_data", city);
	if (snapshot == "no data")
	{
		snapshot = _fetchDataAndUpdateDb(city);
	}
	return snapshot;
}

static std::string getProperValue(const json& value)
{
	if (value.is_string())
	{
		auto res = value.get<std::string>();
		return res.substr(0, res.find_first_of('.'));
	}
	else
	{
		return std::to_string(value.get<int>());
	}
}

static QString formatTextFromJSON(std::string& jsonWeatherData)
{
	json parsedFromString = json::parse(jsonWeatherData);
	std::string city = parsedFromString["city"];
	std::string date = parsedFromString["date"];
	json tempValue = parsedFromString["temp"];
	std::string temp = getProperValue(tempValue);
	std::string time = parsedFromString["time"];
	std::string weather = parsedFromString["weather"];
		
	std::stringstream ss;
	ss << city << '\n' << weather << '\n' << temp << '\n' << date << '\n' << time;

	std::string buildedString = ss.str();

	return QString::fromStdString(buildedString);
}

void FetcherWindow::_displayWeatherData(const std::string& city)
{
	std::string rawOuput;

	auto fetchedDataLabel = findChild<QLabel*>("WeatherData");
	std::string snapshot;
	bool isValidResponse = true;

	try
	{
		snapshot = _getWeatherData(city);
		
		json jdata = json::parse(snapshot);
		std::stringstream ss;
		ss << jdata["date"] << " " << jdata["time"];
		QString date = QString::fromStdString(ss.str());

		if (hasRecentData(date))
		{
			rawOuput = snapshot;
		}
		else
		{
			auto newSnapshot = _fetchDataAndUpdateDb(city);
			rawOuput = newSnapshot;
		}
	}
	catch (const std::runtime_error& e)
	{
		isValidResponse = false;
		_lastWeatherType.clear();
		_lastWeatherType.shrink_to_fit();
	}
	
	auto formattedData = isValidResponse ? formatTextFromJSON(rawOuput) : _defaultDataOutput;
	fetchedDataLabel->setText(formattedData);
}