#include "shellusb.h"
#include "ui_shellusb.h"

shellusb::shellusb(QWidget *parent) :
    QDialog(parent), ui(new Ui::shellusb), model(new QFileSystemModel(this)),lt(new std::list<QString>),iter(lt->rbegin())
{

    ui->setupUi(this);
    this->setWindowTitle("ShellUSB");
    // 맨 처음 경로 저장
    lt->push_back(model->rootPath());

    // treeView에서 사용할 model, index 정의
    treeModel = new QDirModel(this);
    treeModel->setReadOnly(true);
    treeModel->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name );
    QModelIndex index = treeModel->index(QDir::rootPath());

    // treeView
    ui->treeView->setModel(treeModel);
    ui->treeView->setRootIndex(index);
    ui->treeView->setCurrentIndex(index);
    ui->treeView->expand(index);
    ui->treeView->scrollTo(index);
    ui->treeView->resizeColumnToContents(0);

    ui->front_btn->setDisabled(true); //if listFront !=NULL then front_btn enable else then front_btn disabled.
    ui->back_btn->setDisabled(true);

    model->setReadOnly(true);
    ui->tableView->setModel(model);
    ui->tableView->setRootIndex(model->setRootPath("C:/Users/Simong/Desktop"));

    ui->back_btn->setIcon(QIcon("../shellusbv0_2/back.png"));
    ui->front_btn->setIcon(QIcon("../shellusbv0_2/front.png"));
    ui->help_btn->setIcon(QIcon("../shellusbv0_2/help.png"));

    ui->tableView->setAcceptDrops(true);
    ui->tableView->setDragEnabled(true);
    ui->tableView->setDropIndicatorShown(true);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setDefaultDropAction(Qt::MoveAction);
    ui->tableView->setDragDropMode(QAbstractItemView::DragDrop);
}

shellusb::~shellusb()
{
    delete ui;
}

/**
 * @brief 암호화 버튼
 */
void shellusb::on_enc_btn_clicked()
{
    QModelIndex index = ui->tableView->currentIndex();

    int lastdot = model->fileInfo(index).absoluteFilePath().lastIndexOf(".");
    QString duplicated = model->fileInfo(index).absoluteFilePath().mid(lastdot, 9);

    if (!duplicated.compare(".shellUSB"))
    {
        QMessageBox::warning(NULL, "Warning", "This file is already encrypted");
        qDebug() << "duplicated";
        return;
    }

    QByteArray key = crypto.HexStringToByte
            (QInputDialog::getText(NULL, "key", "Enter a key", QLineEdit::Password, NULL, &ok));

    if (!ok)
    {
        qDebug() << "canceled";
        return;
    }

    EncryptThread *encryptthread;
    encryptthread = new EncryptThread(model->fileInfo(index).absoluteFilePath(), key, true);
    connect(encryptthread, SIGNAL(finished()), encryptthread, SLOT(deleteLater()));
    encryptthread->start();
}

/**
 * @brief 복호화 버튼
 */
void shellusb::on_dnc_btn_clicked()
{
    QModelIndex index = ui->tableView->currentIndex();

    int lastdot = model->fileInfo(index).absoluteFilePath().lastIndexOf(".");
    QString duplicated = model->fileInfo(index).absoluteFilePath().mid(lastdot, 9);

    if (duplicated.compare(".shellUSB"))
    {
        QMessageBox::warning(NULL, "Warning", "This file is not encrypted");
        qDebug() << "duplicated";
        return;
    }

    QByteArray key = crypto.HexStringToByte
            (QInputDialog::getText(NULL, "key", "Enter a key", QLineEdit::Password, NULL, &ok));

    if (!ok)
    {
        qDebug() << "canceled";
        return;
    }

    EncryptThread *encryptthread;
    encryptthread = new EncryptThread(model->fileInfo(index).absoluteFilePath(), key, false);
    connect(encryptthread, SIGNAL(finished()), encryptthread, SLOT(deleteLater()));
    encryptthread->start();
}

/**
 * @brief 뒤로 가기 버튼
 */
void shellusb::on_back_btn_clicked()
{
    // 뒤로 이동
    iter++;
    ui->tableView->setRootIndex(model->setRootPath(*iter));
    // front 버튼 활성화
    ui->front_btn->setDisabled(false);
    // 리스트의 맨 앞인 경우 back버튼 비활성화
    if(iter == --lt->rend())
    {
        qDebug() << "back - 리스트의 맨 앞";
        ui->back_btn->setDisabled(true);
    }
}

/**
 * @brief 앞으로 가기 버튼
 */
void shellusb::on_front_btn_clicked()
{
    // 앞으로 이동
    iter--;
    ui->tableView->setRootIndex(model->setRootPath(*iter));
    // back버튼 활성화
    ui->back_btn->setDisabled(false);
    // 리스트의 맨 뒤인 경우 front버튼 비활성화
    if(iter == lt->rbegin())
    {
        ui->front_btn->setDisabled(true);
    }
}

/**
 * @brief tableView 더블 클릭
 * @param index
 */
void shellusb::on_tableView_doubleClicked(const QModelIndex &index)
{
    // 디렉토리를 더블클릭한 경우
    if(model->fileInfo(index).isDir())
    {
        // iter 뒤에 있는 lt의 노드들을 삭제한다.
        int tmp = 0;
        std::list<QString>::reverse_iterator it = lt->rbegin();
        while(it != lt->rend())
        {
            if(it == iter)
                break;
            else
                tmp++;
            it++;
        }
        for(int i=0; i<tmp; i++)
        {
            lt->pop_back();
        }
        // lt 뒤에 새로운 위치 add
        lt->push_back(model->fileInfo(index).absoluteFilePath());
        // 더블클릭한 위치로 이동
        ui->tableView->setRootIndex(model->setRootPath(model->fileInfo(index).absoluteFilePath()));
        // back버튼 활성화/ front버튼 비활성화
        ui->back_btn->setDisabled(false);
        ui->front_btn->setDisabled(true);
    }
    // 파일을 더블클릭한 경우
    else
    {
        QDesktopServices* ds = new QDesktopServices;
        ds->openUrl(QUrl(model->fileInfo(index).absoluteFilePath()));
    }
}

/**
 * @brief treeView 클릭
 * @param treeIndex
 */
void shellusb::on_treeView_clicked(const QModelIndex &treeIndex)
{
    // 선택된 파일 or 디렉토리 경로
    QString path = treeModel->filePath(treeIndex);
    QFileInfo pathInfo(path);
    // 디렉토리 일 경우
    if (pathInfo.isDir())
    {
        // iter뒤에 있는 lt의 노드들을 삭제한다.
        int tmp = 0;
        std::list<QString>::reverse_iterator it = lt->rbegin();
        while(it != lt->rend())
        {
            if(it == iter)
                break;
            else
                tmp++;
            it++;
        }
        for(int i=0; i<tmp; i++)
        {
            lt->pop_back();
        }
        // lt 뒤에 새로운 위치 add
        QString newPath = treeModel->filePath(treeIndex);
        lt->push_back(newPath);
        iter = lt->rbegin();
        // treeView에서 선택한 위치로 tableView 이동
        ui->tableView->setRootIndex(model->setRootPath(path));
        // back버튼 활성화/ front버튼 비활성화
        ui->back_btn->setDisabled(false);
        ui->front_btn->setDisabled(true);
    }
}