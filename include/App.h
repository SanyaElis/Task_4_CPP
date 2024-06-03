#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QPlainTextEdit>
#include "Logger.h"

class App : public QWidget {
Q_OBJECT

public:
    App(QWidget *parent = nullptr);

private slots:
    void processFiles();

private:
    QPlainTextEdit *plainTextEdit;
    Logger logger;

    static std::string removePunctuation(const std::string &word);
    std::map<std::string, std::vector<std::string>> loadSynonyms(const std::string &filename);
    std::string getRandomSynonym(const std::string &word, const std::map<std::string, std::vector<std::string>> &synonyms);
};

#endif // APP_H
