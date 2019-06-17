#ifndef PCH_HPP
#define PCH_HPP

// c
#include <assert.h>
#include <stdint.h>
#include <limits.h>

// c++
#ifdef __cplusplus
// std
#include <functional>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <utility>
#include <random>
// qt
#include <QAction>
#include <QApplication>
#include <QChar>
#include <QClipboard>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QRegExpValidator>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QTableWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QToolBar>
#include <QTranslator>
#include <QUuid>
#endif // __cplusplus

// gmp
#include <gmpxx.h>

// define
typedef uint8_t Byte;

#endif // PCH_HPP
