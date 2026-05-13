#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <QImage>
#include <QBuffer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textLog->setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: Consolas;");
    ui->textLog->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::appendLog(const QString &text)
{
    QString timeStr = QDateTime::currentDateTime().toString("[HH:mm:ss] ");
    ui->textLog->append(timeStr + text);
    QTextCursor cursor = ui->textLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(cursor);
}

bool MainWindow::moveDirectory(const QString &source, const QString &target)
{
    QDir sourceDir(source);
    if (!sourceDir.exists()) return false;

    if (QDir(target).exists()) {
        appendLog(QString("❌ 冲突！目标资源目录已存在，无法移动: %1").arg(target));
        return false;
    }
    return QDir().rename(source, target);
}

bool MainWindow::runGitCommand(const QString &program, const QStringList &arguments)
{
    appendLog(QString("================================"));
    appendLog(QString("> %1 %2").arg(program).arg(arguments.join(" ")));

    QProcess process;
    process.setWorkingDirectory(QCoreApplication::applicationDirPath());

    connect(&process, &QProcess::readyReadStandardOutput, [&](){
        QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) appendLog(output);
    });

    connect(&process, &QProcess::readyReadStandardError, [&](){
        QString errorOut = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        if (!errorOut.isEmpty()) appendLog(errorOut);
    });

    process.start(program, arguments);
    process.waitForStarted();

    while(process.state() == QProcess::Running) {
        process.waitForReadyRead(100);
        QCoreApplication::processEvents();
    }

    return process.exitCode() == 0;
}

// ==========================================
// 图片压缩逻辑 (返回 true 表示进行了压缩)
// ==========================================
bool MainWindow::compressImage(const QString &filePath, QString &logStr)
{
    QFileInfo info(filePath);
    const qint64 MAX_SIZE = 2 * 1024 * 1024; // 2MB限制

    if (info.size() <= MAX_SIZE) {
        return false; // 小于2MB跳过
    }

    QImage img(filePath);
    if (img.isNull()) return false;

    double originalSizeMB = info.size() / 1024.0 / 1024.0;
    QString format = info.suffix().toUpper();
    if (format == "JPG") format = "JPEG";

    QByteArray bufferArray;
    QBuffer buffer(&bufferArray);
    bool sizeOk = false;

    // 区分 PNG 和 JPEG 压缩策略
    if (format == "PNG") {
        // PNG 是无损格式，降低质量参数没用，只能硬着头皮缩放尺寸
        while (img.width() > 600) { // 最多缩小到宽600保底
            img = img.scaled(img.width() * 0.8, img.height() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            bufferArray.clear();
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer, "PNG");
            if (bufferArray.size() <= MAX_SIZE) {
                sizeOk = true;
                break;
            }
        }
    } else {
        // JPEG 等格式：先尝试降低质量，如果不行再配合缩放
        int quality = 85;
        while (quality >= 30) {
            bufferArray.clear();
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer, format.toLocal8Bit().constData(), quality);
            if (bufferArray.size() <= MAX_SIZE) {
                sizeOk = true;
                break;
            }
            quality -= 15;
        }

        // 质量降完了还是大，开始缩放尺寸
        while (!sizeOk && img.width() > 800) {
            img = img.scaled(img.width() * 0.8, img.height() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            bufferArray.clear();
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer, format.toLocal8Bit().constData(), 50); // 以50的质量保存缩放图
            if (bufferArray.size() <= MAX_SIZE) {
                sizeOk = true;
                break;
            }
        }
    }

    // 保存回原路径并记录日志
    if (!bufferArray.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(bufferArray);
            file.close();
            double newSizeMB = bufferArray.size() / 1024.0 / 1024.0;
            logStr = QString("  - %1 (%2 MB -> %3 MB)")
                         .arg(info.fileName())
                         .arg(originalSizeMB, 0, 'f', 2)
                         .arg(newSizeMB, 0, 'f', 2);
            return true;
        }
    }

    return false;
}

void MainWindow::on_btnSubmit_clicked()
{
    ui->btnSubmit->setEnabled(false);
    ui->textLog->clear();
    appendLog("🚀 开始执行自动化整理与提交任务...");

    QString baseDirPath = QCoreApplication::applicationDirPath();
    QDir baseDir(baseDirPath);

    QStringList filters;
    filters << "*.md";
    QFileInfoList fileList = baseDir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

    // 用于汇总报告的日志列表
    QStringList deletedFilesReport;
    QStringList compressedFilesReport;

    for (const QFileInfo &fileInfo : fileList) {
        QString mdName = fileInfo.fileName();
        QString baseName = fileInfo.baseName();
        QFile file(fileInfo.absoluteFilePath());
        QString targetFolder = "随笔文章";

        QString fileContent;

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            fileContent = in.readAll();
            file.close();

            // 解析 Front Matter
            QStringList lines = fileContent.split('\n');
            int dashCount = 0;
            for (const QString &rawLine : lines) {
                QString line = rawLine.trimmed();
                if (line == "---") {
                    dashCount++;
                    if (dashCount == 2) break;
                    continue;
                }
                if (dashCount == 1) {
                    if (line.contains("type: inspiration") || line.contains("layout: inspiration")) {
                        targetFolder = "灵感收集";
                        break;
                    }
                    else if (line.startsWith("project:")) {
                        QString projectName = line.mid(line.indexOf(":") + 1).trimmed();
                        if (!projectName.isEmpty()) {
                            targetFolder = "作品集合/" + projectName;
                        }
                        break;
                    }
                }
            }
        }

        // ==========================================
        // 正则提取 Markdown 引用的图片（兼容 img 和 ![] 写法）
        // ==========================================
        QSet<QString> referencedImages;
        // 匹配 HTML <img> 标签 和 Markdown ![]() 标签
        QRegularExpression re("!\\[.*?\\]\\(([^)]+)\\)|<img[^>]+src=[\"']([^\"']+)[\"']");
        QRegularExpressionMatchIterator i = re.globalMatch(fileContent);

        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            // 提取捕获的路径内容
            QString path = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
            QString decodedPath = QUrl::fromPercentEncoding(path.toUtf8());
            // QFileInfo.fileName() 会自动剥离 "文件夹前缀/"，只保留纯图片名
            referencedImages.insert(QFileInfo(decodedPath).fileName());
        }

        QString sourceAssetFolder = baseDir.absoluteFilePath(baseName);
        if (QDir(sourceAssetFolder).exists()) {
            QDir assetDir(sourceAssetFolder);
            QStringList imgFilters;
            imgFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.webp" << "*.bmp";
            QFileInfoList assetFiles = assetDir.entryInfoList(imgFilters, QDir::Files);

            for (const QFileInfo& asset : assetFiles) {
                QString imgName = asset.fileName();

                // 未被引用 -> 删除并记录
                if (!referencedImages.contains(imgName)) {
                    QFile::remove(asset.absoluteFilePath());
                    deletedFilesReport.append(QString("  - [%1] 删除: %2").arg(baseName).arg(imgName));
                }
                // 被引用 -> 检测是否需要压缩
                else {
                    QString compressLog;
                    if (compressImage(asset.absoluteFilePath(), compressLog)) {
                        compressedFilesReport.append(QString("  - [%1] %2").arg(baseName).arg(compressLog.trimmed()));
                    }
                }
            }
        }

        // ==========================================
        // 目录判断与文件移动 (保持不变)
        // ==========================================
        QString destDirPath = baseDir.absoluteFilePath(targetFolder);
        QDir destDir(destDirPath);
        if (!destDir.exists()) {
            destDir.mkpath(".");
        }

        QString destMdPath = destDir.absoluteFilePath(mdName);
        if (QFile::exists(destMdPath)) {
            appendLog(QString("❌ 冲突！目标文件已存在，无法移动: %1").arg(destMdPath));
            appendLog("🛑 操作已中止，防止覆盖。");
            ui->btnSubmit->setEnabled(true);
            return;
        }

        if (!QFile::rename(fileInfo.absoluteFilePath(), destMdPath)) {
            appendLog(QString("❌ 无法移动文件: %1").arg(mdName));
            ui->btnSubmit->setEnabled(true);
            return;
        }

        if (QDir(sourceAssetFolder).exists()) {
            QString destAssetFolder = destDir.absoluteFilePath(baseName);
            if (QDir(destAssetFolder).exists()) {
                appendLog(QString("❌ 冲突！目标资源文件夹已存在: %1").arg(destAssetFolder));
                appendLog("🛑 操作已中止，防止覆盖。");
                ui->btnSubmit->setEnabled(true);
                return;
            }

            if (!moveDirectory(sourceAssetFolder, destAssetFolder)) {
                ui->btnSubmit->setEnabled(true);
                return;
            }
        }
    }

    // ==========================================
    // 打印汇总报告
    // ==========================================
    appendLog("✅ 文件提取、压缩、转移完毕。");

    if (!deletedFilesReport.isEmpty() || !compressedFilesReport.isEmpty()) {
        appendLog("\n======== 本次图像处理汇总报告 ========");
        if (!deletedFilesReport.isEmpty()) {
            appendLog(QString("🗑️ 共删除了 %1 个未引用的冗余图片：").arg(deletedFilesReport.size()));
            for (const QString &log : deletedFilesReport) appendLog(log);
        }
        if (!compressedFilesReport.isEmpty()) {
            appendLog(QString("📉 共压缩了 %1 个超大图片 ( >2MB )：").arg(compressedFilesReport.size()));
            for (const QString &log : compressedFilesReport) appendLog(log);
        }
        appendLog("======================================\n");
    }

    // Git 操作
    appendLog("⏳ 开始执行 Git 提交操作...");

    if (!runGitCommand("git", QStringList() << "add" << ".")) {
        appendLog("❌ git add 失败，请检查环境！");
        ui->btnSubmit->setEnabled(true);
        return;
    }

    QString commitMsg = "Auto Deploy: 更新文章归档与图片优化";
    runGitCommand("git", QStringList() << "commit" << "-m" << commitMsg);

    if (!runGitCommand("git", QStringList() << "push" << "-u" << "origin" << "main")) {
        appendLog("❌ git push 失败！可能是网络原因，请根据上方日志排查。");
    } else {
        appendLog("🎉 恭喜！全自动化流程执行完毕，已成功推送到远程仓库！");
    }

    ui->btnSubmit->setEnabled(true);
}
