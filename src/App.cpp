#include "../include/App.h"
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <regex>

App::App(QWidget *parent) : QWidget(parent), logger("../logs/app_log.txt") {
    setWindowTitle("Synonym Replacer");
    resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Synonym Replacer", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px; color: #4CAF50;");

    plainTextEdit = new QPlainTextEdit(this);
    plainTextEdit->setReadOnly(true);
    plainTextEdit->setStyleSheet("background-color: #ffffff; padding: 10px; color: #000; border: 1px solid #ccc;");

    QPushButton *button = new QPushButton("Select Files To Process", this);
    button->setStyleSheet("background-color: #4CAF50; color: white; padding: 10px; font-size: 16px; border-radius: 5px;");

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(plainTextEdit);
    mainLayout->addWidget(button);

    connect(button, &QPushButton::clicked, this, &App::processFiles);
}

std::string App::removePunctuation(const std::string &word) {
    std::string cleanedWord;
    std::remove_copy_if(word.begin(), word.end(), std::back_inserter(cleanedWord), [](char c) {
        return std::ispunct(c) || std::isspace(c);
    });
    return cleanedWord;
}

void App::processFiles() {
    try {
        QString wordsFileName = QFileDialog::getOpenFileName(this, "Select Words File", "../data/",
                                                             "Text Files (*.txt);");
        if (wordsFileName.isEmpty()) return;

        QString synonymsFileName = QFileDialog::getOpenFileName(this, "Select Synonyms File", "../data/",
                                                                "Text Files (*.txt);");
        if (synonymsFileName.isEmpty()) return;

        std::ifstream wordsFile(wordsFileName.toStdString());
        std::ifstream synonymsFile(synonymsFileName.toStdString());

        if (!wordsFile.is_open() || !synonymsFile.is_open()) {
            throw std::runtime_error("Unable to open words or synonyms file");
        }

        std::unordered_map<std::string, std::vector<std::string>> synonyms;
        std::string line;
        while (std::getline(synonymsFile, line)) {
            std::istringstream lineStream(line);
            std::string word;
            if (std::getline(lineStream, word, '=')) {
                word = removePunctuation(word);
                std::string synonym;
                while (std::getline(lineStream, synonym, ',')) {
                    synonyms[word].push_back(removePunctuation(synonym));
                }
            }
        }

        std::ostringstream outputContent;
        std::string text((std::istreambuf_iterator<char>(wordsFile)), std::istreambuf_iterator<char>());
        std::regex wordRegex(R"(\b(\w+)([^\w]*)\b)");

        auto wordsBegin = std::sregex_iterator(text.begin(), text.end(), wordRegex);
        auto wordsEnd = std::sregex_iterator();

        std::random_device rd;
        std::mt19937 gen(rd());

        for (std::sregex_iterator it = wordsBegin; it != wordsEnd; ++it) {
            std::smatch match = *it;
            std::string word = match[1].str();
            std::string punctuation = match[2].str();

            std::string cleanedWord = removePunctuation(word);

            if (synonyms.find(cleanedWord) != synonyms.end()) {
                const auto &synList = synonyms[cleanedWord];
                std::uniform_int_distribution<> dis(0, synList.size() - 1);
                std::string synonym = synList[dis(gen)];
                outputContent << synonym << punctuation;
            } else {
                outputContent << word << punctuation;
            }
        }

        QString outputFileName = QFileDialog::getSaveFileName(this, "Save Output File", "../data/",
                                                              "Text Files (*.txt);");
        if (outputFileName.isEmpty()) return;

        std::ofstream outputFile(outputFileName.toStdString());
        if (!outputFile.is_open()) {
            throw std::runtime_error("Unable to open output file");
        }

        outputFile << outputContent.str();
        outputFile.close();
        plainTextEdit->setPlainText(QString::fromStdString(outputContent.str()));
        logger.logMessage("Processing completed successfully", "INFO");
        QMessageBox::information(this, "Success", "Processing completed successfully");
    } catch (const std::exception &e) {
        logger.logMessage(e.what(), "ERROR");
        QMessageBox::critical(this, "Error", e.what());
    }
}
