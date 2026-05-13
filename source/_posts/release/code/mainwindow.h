#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnSubmit_clicked();

private:
    Ui::MainWindow *ui;

    void appendLog(const QString &text);
    bool moveDirectory(const QString &source, const QString &target);
    bool runGitCommand(const QString &program, const QStringList &arguments);

    // 修改：增加日志引用参数，用于汇总报告
    bool compressImage(const QString &filePath, QString &logStr);
};

#endif // MAINWINDOW_H
