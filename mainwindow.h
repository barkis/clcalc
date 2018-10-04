#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
class QTextEdit;
class QLineEdit;
class QTextCursor;
#include <regex>
#include <map>
#include <set>
namespace Ui {
class MainWindow;
}

using ddfunc = double (*) (double);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private slots:
    void updateCursor();
    void onTextChanged();
    void replace(std::string &haystack, const std::string &needle, const std::string newVal);
protected:
    virtual double Calc(QString strEntry) ;
    virtual void checkAssignment(std::string &sEntry);
    virtual bool replaceVariables(std::string &sEntry);
    virtual bool getBrackets(std::string &sEntry);
    virtual void doCalc(std::string &sToCalc, std::string op);
private:
    Ui::MainWindow *ui;
    QTextEdit  *m_pEdit;
    QLineEdit *m_pedtStatus;
    QString currentTextLine();
    QString currentTextLine(int position);
    std::string getRegexError(std::regex_constants::error_type code);
    void updateString(std::string &str, std::smatch match);
    bool isLastLine();
    void moveToEnd(std::string sLine);
    void storeLineEnd();
    void blockUnblock(QObject *obj);
    void SetFuncMap();
    void getMathsFuncs(std::string &sEntry);
    std::string checkFunc(std::string sPrefix,
                          std::string sValue,
                          std::string &sFuncReturned);
    void setConstants();
    int m_MaxCursor = 0;
    bool m_bAssignment;
    std::string m_sVariable;
    std::map<std::string, double> m_mapVariables;

    std::string m_sNumberMatch;
    std::map<std::string, ddfunc> m_mapFuncs;
    std::set<int> m_setLineEnds;
    int m_LastLine = 0;
    bool  m_bLastLine = true;
    int m_LastCol, m_LastPos;
    const QString PROMPT;
    int m_PromptLen;
    std::string m_sFuncPattern;
};

#endif // MAINWINDOW_H
