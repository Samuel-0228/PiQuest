#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>
#include <cmath>

class MathQuizWidget : public QWidget
{
    Q_OBJECT

public:
    MathQuizWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupMainMenu();
        qsrand(QTime::currentTime().msec()); // Seed random
    }

private slots:
    void setDifficulty(int index)
    {
        difficulty = index + 1;
        maxNumber = pow(10, difficulty - 1);
        startQuizButton->setEnabled(true);
        QMessageBox::information(this, "Difficulty Set",
                                 QString("Difficulty level %1 selected (max number: %2).").arg(difficulty).arg(maxNumber));
    }

    void setupProblemTypeMenu()
    {
        clearLayout();
        setWindowTitle("Math Quiz - Choose Problem Type");

        QLabel *titleLabel = new QLabel("Choose Problem Type", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

        QPushButton *addButton = new QPushButton("1. Addition", this);
        connect(addButton, &QPushButton::clicked, [this]()
                { setProblemType(1); });
        QPushButton *subButton = new QPushButton("2. Subtraction", this);
        connect(subButton, &QPushButton::clicked, [this]()
                { setProblemType(2); });
        QPushButton *mulButton = new QPushButton("3. Multiplication", this);
        connect(mulButton, &QPushButton::clicked, [this]()
                { setProblemType(3); });
        QPushButton *divButton = new QPushButton("4. Division", this);
        connect(divButton, &QPushButton::clicked, [this]()
                { setProblemType(4); });
        QPushButton *mixButton = new QPushButton("5. Random Mixture", this);
        connect(mixButton, &QPushButton::clicked, [this]()
                { setProblemType(5); });

        QPushButton *backButton = new QPushButton("Back to Main Menu", this);
        connect(backButton, &QPushButton::clicked, this, &MathQuizWidget::setupMainMenu);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(titleLabel);
        layout->addWidget(addButton);
        layout->addWidget(subButton);
        layout->addWidget(mulButton);
        layout->addWidget(divButton);
        layout->addWidget(mixButton);
        layout->addWidget(backButton);
        layout->addStretch();

        setLayout(layout);
    }

    void setProblemType(int type)
    {
        problemType = type;
        correctCount = 0;
        incorrectCount = 0;
        currentQuestion = 0;
        if (type == 4)
        {
            QMessageBox::information(this, "Division Note", "Round to the nearest integer for division problems.");
        }
        generateQuestion();
    }

    void checkAnswer()
    {
        bool ok;
        int answer = answerEdit->text().toInt(&ok);
        if (!ok)
        {
            QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer.");
            return;
        }

        answerEdit->clear();

        if (answer == correctAnswer)
        {
            correctCount++;
            QString feedback = randomFeedback(true);
            feedbackLabel->setText(feedback);
            feedbackLabel->setStyleSheet("color: green; font-weight: bold;");
            QTimer::singleShot(1500, this, &MathQuizWidget::nextQuestion);
        }
        else
        {
            incorrectCount++;
            QString feedback = randomFeedback(false);
            feedbackLabel->setText(feedback);
            feedbackLabel->setStyleSheet("color: red; font-weight: bold;");
            // Allow retry: just refocus
            answerEdit->setFocus();
        }
    }

    void skipQuestion()
    {
        incorrectCount++;
        QMessageBox::information(this, "Skipped", "Question skipped. Moving to next.");
        nextQuestion();
    }

    void nextQuestion()
    {
        currentQuestion++;
        if (currentQuestion >= 10)
        {
            showResults();
        }
        else
        {
            generateQuestion();
        }
    }

private:
    void setupMainMenu()
    {
        clearLayout();
        setWindowTitle("Math Quiz");

        QLabel *titleLabel = new QLabel("Math Quiz", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");

        QComboBox *difficultyCombo = new QComboBox(this);
        difficultyCombo->addItem("Easy (1)");
        difficultyCombo->addItem("Medium (2)");
        difficultyCombo->addItem("Hard (3)");
        difficultyCombo->addItem("Custom...");
        connect(difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MathQuizWidget::setDifficulty);

        QPushButton *customDiffButton = new QPushButton("Set Custom Difficulty", this);
        connect(customDiffButton, &QPushButton::clicked, [this]()
                {
            bool ok;
            int diff = QInputDialog::getInt(this, "Custom Difficulty", "Enter difficulty level (1=easy, 2=medium, etc.):", 1, 1, 10, 1, &ok);
            if (ok) {
                setDifficulty(diff - 1); // Adjust index
                // Update combo if needed, but for simplicity, skip
            } });

        startQuizButton = new QPushButton("Start Quiz", this);
        startQuizButton->setEnabled(false);
        connect(startQuizButton, &QPushButton::clicked, this, &MathQuizWidget::setupProblemTypeMenu);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(titleLabel);
        layout->addWidget(difficultyCombo);
        layout->addWidget(customDiffButton);
        layout->addWidget(startQuizButton);
        layout->addStretch();

        setLayout(layout);
    }

    void generateQuestion()
    {
        clearLayout();
        setWindowTitle(QString("Math Quiz - Question %1/10").arg(currentQuestion + 1));

        QLabel *questionLabel = new QLabel(QString("Question %1/10").arg(currentQuestion + 1), this);
        questionLabel->setAlignment(Qt::AlignCenter);
        questionLabel->setStyleSheet("font-size: 14px;");

        int num1 = QRandomGenerator::global()->bounded(maxNumber);
        int num2 = QRandomGenerator::global()->bounded(maxNumber);
        int qtype = (problemType == 5) ? (QRandomGenerator::global()->bounded(4) + 1) : problemType;

        char opChar;
        if (qtype == 1)
        {
            correctAnswer = num1 + num2;
            opChar = '+';
        }
        else if (qtype == 2)
        {
            correctAnswer = num1 - num2;
            opChar = '-';
        }
        else if (qtype == 3)
        {
            correctAnswer = num1 * num2;
            opChar = '*';
        }
        else
        { // Division
            while (num2 == 0)
            {
                num2 = QRandomGenerator::global()->bounded(1, maxNumber);
            }
            correctAnswer = static_cast<int>(static_cast<double>(num1) / num2 + 0.5);
            opChar = '/';
        }

        questionText = QString("What is %1 %2 %3?").arg(num1).arg(opChar).arg(num2);
        QLabel *quizLabel = new QLabel(questionText, this);
        quizLabel->setAlignment(Qt::AlignCenter);
        quizLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

        answerEdit = new QLineEdit(this);
        answerEdit->setAlignment(Qt::AlignCenter);
        answerEdit->setStyleSheet("font-size: 16px;");

        QPushButton *submitButton = new QPushButton("Submit", this);
        connect(submitButton, &QPushButton::clicked, this, &MathQuizWidget::checkAnswer);

        QPushButton *skipButton = new QPushButton("Skip", this);
        connect(skipButton, &QPushButton::clicked, this, &MathQuizWidget::skipQuestion);

        feedbackLabel = new QLabel("", this);
        feedbackLabel->setAlignment(Qt::AlignCenter);

        // Bind Enter key to submit
        connect(answerEdit, &QLineEdit::returnPressed, this, &MathQuizWidget::checkAnswer);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(questionLabel);
        layout->addWidget(quizLabel);
        layout->addWidget(answerEdit);
        layout->addWidget(submitButton);
        layout->addWidget(skipButton);
        layout->addWidget(feedbackLabel);
        layout->addStretch();

        setLayout(layout);
        answerEdit->setFocus();
    }

    void showResults()
    {
        double percentage = (correctCount / 10.0) * 100.0;
        QString resultText = QString("You answered %1 correctly and %2 incorrectly.\nYour score: %3%")
                                 .arg(correctCount)
                                 .arg(incorrectCount)
                                 .arg(percentage, 0, 'f', 1);

        QString advice;
        if (percentage < 75.0)
        {
            advice = "Please ask your teacher for extra help.";
        }
        else
        {
            advice = "Congratulations, you are ready to go to the next level!";
        }

        QMessageBox::information(this, "Quiz Results", resultText + "\n\n" + advice);

        // Next student prompt
        if (QMessageBox::question(this, "Next Student", "--- Next student, get ready! ---\n\nStart new quiz?") == QMessageBox::Yes)
        {
            setupProblemTypeMenu();
        }
        else
        {
            qApp->quit();
        }
    }

    QString randomFeedback(bool correct)
    {
        QStringList feedbacks;
        if (correct)
        {
            feedbacks << "Very good!" << "Excellent!" << "Nice work!" << "Keep up the good work!";
        }
        else
        {
            feedbacks << "No. Please try again." << "Wrong. Try once more." << "Don't give up!" << "No. Keep trying.";
        }
        return feedbacks[QRandomGenerator::global()->bounded(feedbacks.size())];
    }

    void clearLayout()
    {
        QLayout *layout = this->layout();
        if (layout)
        {
            QLayoutItem *child;
            while ((child = layout->takeAt(0)) != nullptr)
            {
                if (child->widget())
                {
                    child->widget()->deleteLater();
                }
                delete child;
            }
        }
    }

    // Members
    int difficulty = 1;
    int maxNumber = 10;
    int problemType = 1;
    int correctCount = 0;
    int incorrectCount = 0;
    int currentQuestion = 0;
    int correctAnswer = 0;
    QString questionText;
    QPushButton *startQuizButton = nullptr;
    QLineEdit *answerEdit = nullptr;
    QLabel *feedbackLabel = nullptr;
};

#include "main.moc" // For qmake

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MathQuizWidget widget;
    widget.show();
    return app.exec();
}