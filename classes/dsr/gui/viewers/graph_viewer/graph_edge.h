/*
 * Copyright 2018 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRAPHEDGE_H
#define GRAPHEDGE_H

#include <QGraphicsItem>
#include <QContextMenuEvent>
#include <QTableWidget>
#include <memory>
#include <QPen>
#include <cppitertools/zip.hpp>
#include <QLabel>
#include "../../dsr_gui.h"
#include <qmat/QMatAll>
#include <QHeaderView>
#include <cppitertools/range.hpp>


class DoRTStuff : public  QTableWidget
{
Q_OBJECT
public:
    DoRTStuff(std::shared_ptr<DSR::DSRGraph> graph_, const DSR::IDType &from_, const DSR::IDType &to_, const std::string &label_) :
            graph(graph_), from(from_), to(to_), label(label_)
    {
        qRegisterMetaType<DSR::IDType>("DSR::IDType");
//      qRegisterMetaType<DSR::AttribsMap>("DSR::Attribs");

        std::optional<Node> n = graph->get_node(from);
        std::optional<Node> n2 = graph->get_node(to);
        this->horizontalHeader()->hide();
        this->verticalHeader()->hide();

        if (n.has_value() &&  n2.has_value())
        {
            //TODO: Comprobar esto
            //setWindowModality(Qt::ApplicationModal);
            setWindowTitle("RT: " + QString::fromStdString(n.value().name()) + "(" + QString::fromStdString(n.value().type()) + ") to " + QString::fromStdString(n2.value().name()) + "(" + QString::fromStdString(n2.value().type()) + ")");
            setColumnCount(4);
            setRowCount(9);
            setHorizontalHeaderLabels(QStringList{"a", "b", "c", "d", "", "T", "", "R"});
            setVerticalHeaderLabels(QStringList{"a", "b", "c", "d"});
            horizontalHeader()->setStretchLastSection(true);
            resizeRowsToContents();
            resizeColumnsToContents();
            int width = (this->model()->columnCount() - 1) + this->verticalHeader()->width();
            int height = (this->model()->rowCount() - 1) + this->horizontalHeader()->height();
            for(int column = 0; column < this->model()->columnCount(); column++)
                width = width + this->columnWidth(column);
            for(int row = 0; row < this->model()->rowCount(); row++)
                height = height + this->rowHeight(row);
            this->setMinimumWidth(width);
            this->setMinimumHeight(height);
            QObject::connect(graph.get(), &DSR::DSRGraph::update_edge_signal, this, &DoRTStuff::drawSLOT);
            drawSLOT(from, to);
            show();
            std::cout << __FILE__ << " " << __FUNCTION__ << " End ofDoRTStuff Constructor " << std::endl;
            resize_widget();
        }
    };

    void closeEvent (QCloseEvent *event) override
    {
        disconnect(graph.get(), 0, this, 0);
        QTableWidget::closeEvent(event);
    };

    void resizeEvent(QResizeEvent* event)
    {
        const auto &columns = columnCount();
        for(auto &&index : iter::range(columns))
            setColumnWidth(index, ((width()-verticalHeader()->width())/columns)-2);
    }

public slots:
    void drawSLOT(const std::int32_t &from_, const std::int32_t &to_)
    {
        std::cout << __FILE__ << " " << __FUNCTION__ << std::endl;
        if (from==from_ and to==to_)     //ADD LABEL
        {
            try {
                std::optional<Node> node = graph->get_node(from);
                if (node.has_value()) {
                    auto mat = graph->get_edge_RT_as_RTMat(graph->get_edge_RT(node.value(), to));
                    // draw RT values
                    for (auto i : iter::range(mat.nRows()))
                        for (auto j : iter::range(mat.nCols()))
                            if (item(i, j)==0)
                                this->setItem(i, j, new QTableWidgetItem(QString::number(mat(i, j), 'f', 5)));
                            else
                                this->item(i, j)->setText(QString::number(mat(i, j), 'f', 5));
                    // draw translation values
                    auto trans = mat.getTr();
                    std::vector<QString> ts{"tx", "ty", "tz"};
                    std::vector<QString> rs{"rx", "ry", "rz"};
                    std::vector<float> rot{mat.getRxValue(), mat.getRyValue(), mat.getRzValue()};
                    for (auto i: iter::range(3)) {
                        if (this->item(4, i)==0) {
                            auto green = new QTableWidgetItem();
                            green->setBackground(QBrush(QColor("lightGreen")));
                            this->setItem(4, i, green);
                        }
                        if (this->item(5, i)==nullptr)
                            this->setItem(5, i, new QTableWidgetItem(ts[i]));
                        else
                            this->item(5, i)->setText(ts[i]);
                        if (this->item(6, i)==nullptr)
                            this->setItem(6, i, new QTableWidgetItem(QString::number(trans[i], 'f', 5)));
                        else
                            this->item(6, i)->setText(QString::number(trans[i], 'f', 5));
                        if (this->item(7, i)==nullptr)
                            this->setItem(7, i, new QTableWidgetItem(rs[i]));
                        else
                            this->item(7, i)->setText(rs[i]);
                        if (this->item(8, i)==0)
                            this->setItem(8, i, new QTableWidgetItem(QString::number(rot[i], 'f', 5)));
                        else
                            this->item(8, i)->setText(QString::number(rot[i], 'f', 5));
                    }
                }
            }
            catch (const std::exception& e) {
                std::cout << "Exception: " << e.what() << " Cannot find attribute named RT in edge going " << from
                          << " to " << to << std::endl;
            }
        }
        this->resize_widget();
    }
private:
    std::shared_ptr<DSR::DSRGraph> graph;
    int from, to;
    std::string label;
    void resize_widget()
    {
        resizeRowsToContents();
        resizeColumnsToContents();
        int width = (this->model()->columnCount() - 1) + this->verticalHeader()->width() + 4;
        int height = (this->model()->rowCount() - 1) + this->horizontalHeader()->height() ;
        for(int column = 0; column < this->model()->columnCount(); column++)
            width = width + this->columnWidth(column) + 2;
        for(int row = 0; row < this->model()->rowCount(); row++)
            height = height + this->rowHeight(row) ;
        if(abs(width-this->width())>1 or abs(height-this->height())>1) {
            this->setFixedSize(QSize(width, height));
        }
    }
};



class GraphEdge : public QObject, public QGraphicsLineItem, public std::enable_shared_from_this<GraphEdge>
{
Q_OBJECT
    Q_PROPERTY(int edge_pen READ _edge_pen WRITE set_edge_pen)
public:
    GraphEdge(GraphNode *sourceNode, GraphNode *destNode, const QString &edge_name);
    GraphNode *sourceNode() const;
    GraphNode *destNode() const;
    void adjust(GraphNode* node= nullptr, QPointF pos=QPointF());
    int type() const override { return Type; }
    QString getTag() const { return tag;};
    int _edge_pen();
    void set_edge_pen(const int with);
    void change_detected();

protected:
    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    GraphNode *source, *dest;  //CAMBIAR A FROM TO
    qreal arrowSize;
    QPointF sourcePoint;
    QPointF destPoint;
    QString tag;
    QGraphicsTextItem *rt_values = nullptr;
    QTableWidget *label = nullptr;
    int edge_width;
    QPropertyAnimation* animation;
    QPolygonF tag_polygon;

};


#endif // GRAPHEDGE_H
