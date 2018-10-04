#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextLine>
#include <QTextBlock>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QMessageBox>

#include <boost/algorithm/string.hpp>

#include <regex>
#include <string>
#include <memory>
#include <cmath>

template<class T>std::string to_string(T d) {
    std::stringstream os;
    os << d;
    return os.str();
}

double stod(const std::string &strin)   {
    std::stringstream ss(strin);
    double RetVal;
    ss >> RetVal;
    return RetVal;
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_sNumberMatch( R"_((\d*\.?\d*))_"),
    PROMPT(">>")

{
    SetFuncMap();
    setConstants();
    m_PromptLen = PROMPT.length();
    ui->setupUi(this);
    m_pEdit = ui->edtEntry;
    m_pedtStatus = ui->edtStatus;
    m_pEdit->setFocus();
    //  m_pEdit->setText(PROMPT);
    m_LastPos = m_PromptLen - 1;
    QTextCursor cursor = m_pEdit->textCursor();
    cursor.insertText(PROMPT);
    QInputMethod *keyboard = QGuiApplication::inputMethod();
    keyboard->show();

    connect(m_pEdit,SIGNAL(cursorPositionChanged()), this, SLOT(updateCursor()));
    //connect(m_pEdit, SIGNAL(selectionChanged()), this, SLOT(updateCursor()));
    connect(m_pEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetFuncMap()   {
    m_mapFuncs.insert(std::make_pair<std::string, ddfunc>("sin",&std::sin));
    m_mapFuncs.insert(std::make_pair<std::string, ddfunc>("cos",&std::cos));
    m_mapFuncs.insert({"tan", &std::tan});
    m_mapFuncs.insert({"asin", &std::asin});
    m_mapFuncs.insert({"acos", &std::acos});
    m_mapFuncs.insert({"atan", &std::atan});
    m_mapFuncs.insert({"sinh", &std::sinh});
    m_mapFuncs.insert({"cosh", &std::cosh});
    m_mapFuncs.insert({"tanh", &std::tanh});
    m_mapFuncs.insert({"asinh", &asinh});
    m_mapFuncs.insert({"acosh", &acosh});
    m_mapFuncs.insert({"atanh", &atanh});
    m_mapFuncs.insert({"exp", &std::exp});
    m_mapFuncs.insert({"log", &std::log});
    m_mapFuncs.insert({"ln", &std::log});
    m_mapFuncs.insert({"log10", &std::log10});
    m_mapFuncs.insert({"fabs", &std::fabs});
    m_mapFuncs.insert({"sqrt", &std::sqrt});
    m_mapFuncs.insert({"ceil", &std::ceil});
    m_mapFuncs.insert({"floor", &std::floor});
    m_mapFuncs.insert({"round", &round});


    /*  std::stringstream ss("(?:");
    for(auto iter = m_mapFuncs.begin();iter != m_mapFuncs.end();++iter)    {
        ss << iter->first + "|";
    }

    ss.putback('|');
    ss << ")";
    ss << "\\((.*)\\)";

    m_sFuncPattern = ss.str();*/
}

void MainWindow::setConstants()  {
    m_mapVariables.insert({"pi",M_PI});
    m_mapVariables.insert({"e",M_E});
}

//slot for cursorPostionChanged
void MainWindow::updateCursor()  {
    QTextCursor cursor = m_pEdit->textCursor();
    if(!isLastLine())   {
        m_bLastLine = false;
        QString strEntry = currentTextLine();
        moveToEnd(strEntry.toStdString());
    }
    else    {
        /*xxx if(m_LastCol)   {
            QString strText = currentTextLine(m_LastPos);
            Calc(strText);
        }*/
        m_LastPos = cursor.position();
        m_LastCol = cursor.columnNumber();
    }
    //m_pedtStatus->setText(QString::number(lineNo));

}

void MainWindow::blockUnblock(QObject *obj) {
    obj->blockSignals(! obj->signalsBlocked());
}

void MainWindow::onTextChanged()    {
    QString strText = m_pEdit->toPlainText();
    m_bLastLine = isLastLine();
    if(strText.size())  {
        //blockUnblock(m_pEdit);
        const QSignalBlocker blocker(m_pEdit);
        QChar cLast = strText.back();
        if(cLast == '\n')   {
            QString strEntry = currentTextLine();
            storeLineEnd();    //add position of line end to m_setLineEnds
            /*xxx   if(m_bLastLine)  {
                QTextCursor cursor = m_pEdit->textCursor();
                cursor.insertText(PROMPT);
                //   moveToEnd(strEntry.toStdString());
            }*/
            double result = Calc(strEntry);
            QTextCursor cursor = m_pEdit->textCursor();
            int col = cursor.columnNumber();
            [[maybe_unused]]int pos = cursor.position();
            if(col) {
                cursor.insertText("\n");
            }
            cursor.insertText(to_string(result).c_str());
            cursor.insertText("\n" + PROMPT);

        }
        else    {
            if(! m_bLastLine)  {
                //  cursor.deleteChar();
            }
        }
        //   blockUnblock(m_pEdit);
    }
}

void MainWindow::moveToEnd(std::string sLine)   {
    try {
        blockUnblock(m_pEdit);
        QTextCursor cursor = m_pEdit->textCursor();
        //  cursor.deleteChar();
        QTextCursor *pNewCursor = new QTextCursor(m_pEdit->document());
        pNewCursor->movePosition(QTextCursor::End);

        m_pEdit->setTextCursor(*pNewCursor);
        //  cursor.insertText("\n" + QString(sLine.c_str()));
        QString strText = m_pEdit->toPlainText();
        if(strText.back() != '\n')  {
            pNewCursor->insertText("\n");
        }
        //remove newline if it's there
        if(sLine[sLine.size() - 1] == '\n') {
            sLine = sLine.substr(0, sLine.size()  - 1);
        }
        sLine = PROMPT.toStdString() + sLine;
        pNewCursor->insertText(sLine.c_str());
        blockUnblock(m_pEdit);
        m_pEdit->verticalScrollBar()->setValue(m_pEdit->verticalScrollBar()->maximum());
    } catch (std::exception ex) {
        std::string s = ex.what();
    }
}

void MainWindow::storeLineEnd() {
    QTextCursor cursor = m_pEdit->textCursor();
    int pos = cursor.position();
    int lastPos = m_setLineEnds.size() ? *(--m_setLineEnds.end()): -1;
    if(pos > lastPos)   {
        m_setLineEnds.insert(pos);
    }
}
bool MainWindow::isLastLine()   {
    int lastPos;
    QTextCursor cursor = m_pEdit->textCursor();
    int pos = cursor.position();
    // int LastPos = m_vLineEnds[m_vLineEnds.size() - 1];
    if(m_setLineEnds.size())    {
        lastPos = *(--m_setLineEnds.end());
    }
    else    {
        lastPos = 0;
    }
    return pos >= lastPos;
}


//overloaded method
QString MainWindow::currentTextLine()
{
    QTextCursor cursor = m_pEdit->textCursor();
    int col = cursor.columnNumber();
    int cursorPos = cursor.position();
    int pos = cursorPos - 1;
    //if on blank line at end need to move extra space to get past \n
    //  if(isLastLine() && (col == 0))   {
    if(isLastLine() && !col )   {
        --pos;
    }
    if(pos < 0) {
        pos = 0;
        // pos = m_PromptLen;
    }
    QString strText = m_pEdit->toPlainText();
    int adj = col == 0 ? 1 : 0;
    int start = strText.lastIndexOf('\n',pos - adj - 1) + adj;
    if(start == -1) {
        start = 0;
        // start = m_PromptLen;
    }
    int len = pos - start + 2;
    QString strRetVal = strText.mid(start, len);
    if(strRetVal.startsWith(PROMPT))  {
        strRetVal = strRetVal.right(strRetVal.length() - m_PromptLen);
    }
    return strRetVal;
}

QString MainWindow::currentTextLine(int position)   {
    QTextCursor cursor;
    QString strText = m_pEdit->toPlainText();
    if(strText[position] == '\n')   {
        --position;
    }
    cursor.setPosition(position);

    int start = strText.lastIndexOf(PROMPT,position) + m_PromptLen;
    int len = strText.length() - start;
    QString strRetVal = strText.mid(start,len);
    cursor.insertText("\n");
    return strRetVal;
}

double MainWindow::Calc(QString strEntry)    {
    std::string sNum(strEntry.toStdString());
    boost::algorithm::to_lower(sNum);
    checkAssignment(sNum);

    if(replaceVariables(sNum))   {
        //  getMathsFuncs(sNum);
        if(! getBrackets(sNum)) {
            // sNum = "0.0";
        }
    }
    return stod(sNum);
}

void MainWindow::checkAssignment(std::string &sEntry)   {
    try {

        std::string sAssignmentMatch("(^\\s*([A-za-z_]\\w*)\\s*=\\s*)");
        //	sAssignmentMatch = "^\\s*\\w*";
        std::regex rxAssignment(sAssignmentMatch);
        std::smatch assignmentMatch;
        if((m_bAssignment = std::regex_search(sEntry,assignmentMatch,rxAssignment)))   {
            std::string sAssignment = assignmentMatch[1];
            m_sVariable = assignmentMatch[2];
            sEntry = sEntry.substr(sAssignment.size());
            // m_pedtStatus->setText(QString(sEntry.c_str()));
            //  Memo1->Lines->Add(sEntry.c_sEntry());
        }
    }
    catch(std::regex_error ex)  {
        std::string message(std::string(ex.what()) + std::string(": ") +
                            getRegexError(ex.code()));
        m_pedtStatus->setText(message.c_str());

    }

}

void MainWindow::getMathsFuncs(std::string &sEntry) {
    for(auto iter = m_mapFuncs.begin();iter != m_mapFuncs.end(); ++iter)    {

    }

}

bool MainWindow::replaceVariables(std::string &sEntry) {
    std::regex rxVars("([A-Za-z_]\\w*)");
    std::smatch varMatches;
    bool bContinue = true;
    bool bRetVal = true;
    std::string sMatch;
    std::string sTempEntry(sEntry);
    while(bContinue)    {
        if((bContinue = std::regex_search(sTempEntry, varMatches, rxVars))) {
            sMatch = varMatches[1];
            //check for maths functions
            unsigned int matchCount = m_mapFuncs.count(sMatch);
            if(matchCount == 0 )  {
                try {

                    double val = m_mapVariables.at(sMatch);
                    replace(sEntry, sMatch, to_string(val));
                    replace(sTempEntry, sMatch, to_string(val));
                } catch(std::out_of_range ex) {
                    bRetVal = false;
                    m_pedtStatus->setText(QString("Invalid variable ") + sMatch.c_str());
                    break;
                }
            }
            else    {
                replace(sTempEntry, sMatch, "");
            }
        }
    }
    return bRetVal;
}

//calculation method is called from here
bool MainWindow::getBrackets(std::string &sEntry)  {
    //find pairs of parentheses
    //    std::regex reg("([A-Za-z]*)\\s*\\((.+)\\)");
    std::regex reg("\\(([^)]+)\\)");
    std::cmatch mr;
    std::regex_search(sEntry.c_str(), mr, reg);

    //if at innermost brackets do the calculation
    if (mr.empty()) {
        //perform multiplications and division
        doCalc(sEntry, "\\/\\*");
        std::string sSubEntry = mr[1];
        // doCalc(sSubEntry, "\\/\\*");

        //perform additions and subtractions
        doCalc(sEntry,"\\-+");


        double res = stod(sEntry);
        QString strRes(sEntry.c_str());
        /*  if(res == 0.0) {
            throw std::runtime_error(std::string("Invalid result: ") + sEntry);
        }*/
        m_mapVariables["_"] = res;
        if(m_bAssignment)  {
            m_mapVariables[m_sVariable] = res;
        }
    }
    else {
        for (unsigned int i = 1; i < mr.size(); ++i) {
            std::string s(mr[i]);
            //  std::string s(mr[1]);
            std::string sFuncReturned;
            try {
                getBrackets(s);
                s =  checkFunc(mr.prefix(), s,  sFuncReturned);

            } catch (std::exception &ex) {
                m_pedtStatus->setText(ex.what());
            }
            std::string sWholeMatch = mr[0];
            if(sFuncReturned.size())    {
                sWholeMatch = sFuncReturned + sWholeMatch;
            }
            replace(sEntry, sWholeMatch, s);
        }
        getBrackets(sEntry);
    }
    return true;
}

void MainWindow::doCalc(std::string &sToCalc, std::string op) {

    std::string sOpen("(");
    std::string sClose(")");
    std::smatch opMatches;

    std::string sPattern =  m_sNumberMatch + "\\s*([" + op + "])\\s*" + m_sNumberMatch;
    // std::string sPattern =  "\s" ;
    //Memo1->Lines->Add(sPattern.c_str());
    try {
        std::regex rxOp(sPattern);

        bool bContinue = true;
        while(bContinue)    {
            if((bContinue = std::regex_search(sToCalc, opMatches, rxOp))) {
                updateString(sToCalc, opMatches);
            }
        }

    }
    catch (std::regex_error &ex) {
        std::string message(std::string(ex.what()) + std::string(": ") +
                            getRegexError(ex.code()));
    }


}

void MainWindow::replace(std::string &haystack, const std::string &needle, const std::string newVal)  {
    std::size_t pos = haystack.find(needle, 0);
    if(pos < haystack.size())   {
        haystack.replace(pos, needle.size(), newVal);
    }

}

void MainWindow::updateString(std::string &str, std::smatch match)  {
    std::stringstream ss;
    for(unsigned int i = 0;i < match.size();++i)   {
        ss << "match " << i << " = " << match[i];
        ss.str("");
    }
    std::string sWholeMatch = match[0];
    std::string sAssignment = m_bAssignment ? "Assignment True": "Assignment False";
    std::string sNum1 = match[1];
    std::string sOp = match[2];
    std::string sNum2 = match[3];
    double num1 = ::stod(sNum1);
    double num2 = ::stod(sNum2);
    double res = -1;
    switch(sOp[0])   {
    case '*':
        res = num1 * num2;
        break;
    case '/':
        res = num1 / num2;
        break;
    case '+':
        res = num1 + num2;
        break;
    case '-':
        res = num1 - num2;
        break;
    case '%':
        res = int(num1) % int(num2);
        break;
    default:
        throw std::runtime_error("Invalid operator: " + sOp);
    }
    replace(str, sWholeMatch, to_string(res));
}

std::string MainWindow::checkFunc(std::string sPrefix,  std::string sValue, std::string &sFuncReturned)  {
    char szBuff[11];
    szBuff[10] = '\0';
    int count = 9;
    std::string sRetVal;
    for(std::string::iterator iter = sPrefix.end() - 1; iter != sPrefix.begin() - 1; --iter) {
        if((count < 0) || ! isalpha(*iter))   {
            break;
        }
        szBuff[count--] = *iter;
    }
    if((count >= -1) && (count < 9))    {
        std::string sFunc(&szBuff[count + 1]);
        if(m_mapFuncs.count(sFunc)) {
            sFuncReturned = sFunc;
            ddfunc theFunc = m_mapFuncs[sFunc];
            double dRes = theFunc(stod(sValue));
            sValue =  to_string(dRes);
         }
    }
    sRetVal = sValue;
    return sRetVal;
}

std::string MainWindow::getRegexError(std::regex_constants::error_type code) {
    std::string sRetVal;
    switch (code) {
    case std::regex_constants::error_collate:
        sRetVal = "Collate";
        break;
    case std::regex_constants::error_ctype:
        sRetVal = "Invalide character class name";
        break;
    case std::regex_constants::error_escape:
        sRetVal = "Invalid escaped character";
        break;
    case std::regex_constants::error_backref:
        sRetVal = "Invalid back reference";
        break;
    case std::regex_constants::error_brack:
        sRetVal = "Mismatched brackets";
        break;
    case std::regex_constants::error_paren:
        sRetVal = "Mismatched parentheses";
        break;
    case std::regex_constants::error_brace:
        sRetVal = "Mismatched braces";
        break;
    case std::regex_constants::error_badbrace:
        sRetVal = "Invalid range between braces";
        break;
    case std::regex_constants::error_range:
        sRetVal = "Invalid character range";
        break;
    case std::regex_constants::error_space:
        sRetVal = "Insufficient memory";
        break;
    case std::regex_constants::error_badrepeat:
        sRetVal = "Repeat specifier not preceded by valid expression";
        break;
    case std::regex_constants::error_complexity:
        sRetVal = "Complexity exceeded preset level";
        break;
    default:
        sRetVal = "Unknown error";

    }
    return sRetVal;
}
