
#include "filecontrol.h"
#include <QDebug>

FileControl::FileControl(QString _name):fname(_name), pd(new progdialog){

}
FileControl::~FileControl(){
    delete pd;
}

void FileControl::file2enc(){
    bool ok; //QInputDialog button flag.
    qDebug()<<"enc "+this->fname;
    //The line 15~22 check the file exists. if exists is ".shell" then encrypted. return this function.
    int s = this->fname.lastIndexOf(".");
    QString tmp = this->fname.right(this->fname.size()-s);
    if(tmp==".shell"){
        qDebug()<<"this file is encrypted.";
        QMessageBox::warning(NULL,"Warning","This file is encrypted.");
        return;
    }

    this->setFileName(this->fname);
    QByteArray key = crypto.HexStringToByte
            (QInputDialog::getText(NULL, "key", "Enter a key", QLineEdit::Password, NULL, &ok));
    if(ok)
        pd->show();
    else{
        qDebug()<<"cancel.";
        return;
    }

    if (!this->open(this->ReadOnly)){
         QMessageBox::warning(NULL,"Warning",this->fname+" Could not open.");
         qDebug() << "Could not open read file";
         return;
    }


    QByteArray data = this->readAll();
    data.append(tmp); // append file exists.
    this->close();
    this->fname.remove(tmp);
    this->setFileName(this->fname + ".shell");

    if (!this->open(this->WriteOnly)){
        QMessageBox::warning(NULL,"Warning",this->fname+".shell file Could not open.");
        qDebug() << "Could not open write file";
        return;
    }


    QByteArray encrypted = crypto.Encrypt(data, key);

    this->write(encrypted);

    this->flush();
    this->close();
    qDebug()<<"enc " + this->fname;
    //remove the file.
    this->setFileName(this->fname+tmp);
    this->remove();
}

void FileControl::file2dec(){
    bool ok;
    int s = this->fname.lastIndexOf(".");
    QString tmp = this->fname.right(this->fname.size()-s);
    if(tmp!=".shell"){
        qDebug()<<"this file is not encrypted.";
        QMessageBox::warning(NULL,"Warning","This file is not encrypted.");
        return;
    }
    qDebug()<<"dec 1 : "+this->fname;
    this->setFileName(this->fname);

    QByteArray key = crypto.HexStringToByte
            (QInputDialog::getText(NULL, "key", "Enter a key", QLineEdit::Password, NULL, &ok));
    if(ok)
        pd->show();
    else{
        qDebug()<<"cancel.";
        return;
    }

    if (!this->open(this->ReadOnly)){
        qDebug() << "Could not open read file";
        return;
    }

    QByteArray data = this->readAll();

    this->close();
    this->fname.remove(tmp);
    QString filename = this->fname+"~";

    qDebug()<<"dec 2 : " + filename;
    this->setFileName(filename);

    if (!this->open(this->WriteOnly)){
        qDebug() << "Could not open write file";
        return;
    }

    QByteArray decrypted = crypto.Decrypt(data, key);

    s = decrypted.lastIndexOf(".");
    tmp = decrypted.right(decrypted.size()-s);
    this->write(decrypted);

    this->flush();
    this->close();
    qDebug()<<"dec 3 : " + this->fname+tmp;
    this->rename(this->fname + tmp);

    this->setFileName(this->fname+".shell");
    this->remove();
}
