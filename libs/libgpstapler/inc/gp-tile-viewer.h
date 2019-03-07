/*
 * Libgpstapler is a graphical tiles manipulation library.
 *
 * Copyright 2013 Sergey Volkhin.
 *
 * This file is part of Libgpstapler.
 *
 * Libgpstapler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Libgpstapler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Libgpstapler. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file tile-viewer.h
 *
 * \author Sergey Volkhin
 * \date 29.08.2014
 *
 * \defgroup GpTileViewer GpTileViewer - виджет для отображения одной плитки, сгенерированной объектом типа GpTiler.
 * @{
 *
*/

#ifndef __GP_TILE_VIEWER_H__
#define __GP_TILE_VIEWER_H__

#include <gtk/gtk.h>
#include <cairo.h>
#include "gp-tiler.h"

G_BEGIN_DECLS


#define G_TYPE_GP_TILE_VIEWER          (gp_tile_viewer_get_type())
#define GP_TILE_VIEWER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),    G_TYPE_GP_TILE_VIEWER, GpTileViewer))
#define GP_TILE_VIEWER_CLASS(vtable)   (G_TYPE_CHECK_CLASS_CAST((vtable),    G_TYPE_GP_TILE_VIEWER, GpTileViewerClass))
#define GP_TILE_VIEWER_IS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),    G_TYPE_GP_TILE_VIEWER))
#define GP_TILE_VIEWER_CLASS_IS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),     G_TYPE_GP_TILE_VIEWER))
#define GP_TILE_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_GP_TILE_VIEWER, GpTileViewerClass))


typedef struct _GpTileViewer GpTileViewer;
typedef struct _GpTileViewerClass GpTileViewerClass;
typedef struct _GpTileViewerPriv GpTileViewerPriv;

GType gp_tile_viewer_get_type( void );


struct _GpTileViewer
{
  GtkVBox parent;

  // private.
  GpTileViewerPriv *priv;
};

/*! Описание класса GpTileViewer.*/
struct _GpTileViewerClass
{
  GtkVBoxClass parent_class; /*!< Родительский класс.*/

  /// \name Сигналы.
  /// @{
    /*!
     * Сигнал "refreshing-started", сигнализирующий о том,
     * что GpTileViewer начал по таймауту опрашивать GpTiler на предмет готовности
     * нужной GpTileViewer'у (и генерируемой в данный момент) плитки.
     */
    void (*refreshing_started) ( GpTileViewer *tile_viewer);

    /*!
     * Сигнал "refreshing-done", сигнализирующий о том,
     * что требуемая плитка отрисована и, если был запущен таймаут с опросом GpTiler'а
     * на предмет готовности нужной GpTileViewer'у плитки,
     * то GpTileViewer закончил опрос по таймауту.
     * Соответсвенно может прийти и без "refreshing-started", если плитка
     * получена из GpTiler'а сразу при первом запросе, без запуска таймаута.
     */
    void (*refreshing_done) ( GpTileViewer *tile_viewer);
  /// @}
};



/// Создание объекта.
///
/// Создаст виджет для отображения одной плитки, сгенерированной объектом типа GpTiler.
///
/// \param tiler - указатель на объект генератора плиток GpTiler.
///
/// \return указатель на объект.
GpTileViewer *gp_tile_viewer_new( GpTiler *tiler );



/// Заново загружает текущую плитку из генератора плиток GpTiler.
///
/// Поведение аналогично поведению виджета в случае
/// нажатия на кнопку GTK_STOCK_REFRESH на тулбаре виджета.
///
/// \param tile_viewer - указатель на объект для отображения плитки.
void gp_tile_viewer_refresh( GpTileViewer *tile_viewer );



/// Показывает плитку \a tile на виджете \a tile_viewer.
///
/// \param tile_viewer - указатель на объект для отображения плитки;
/// \param tile - описание плитки для показа.
void gp_tile_viewer_show_tile( GpTileViewer *tile_viewer, GpTile tile );



/// Устанавливает "домашнюю" плитку.
///
/// В любой момент можно перейти к отображению "домашней" плитки
/// по клику на кнопку GTK_STOCK_HOME на тулбаре виджета.
/// Если пользователь еще не установил "домашнюю" плитку вручную,
/// то домашней по умолчанию назначается первая отображенная
/// с помощью gp_tile_viewer_show_tile() плитка.
void gp_tile_viewer_set_home_tile( GpTileViewer *tile_viewer, GpTile tile );



/// Позволяет показать или скрыть тулбар с кнопками.
///
/// \param tile_viewer - указатель на объект для отображения плитки;
/// \param toolbar_visible - TRUE, если нужно показать тулбар, FALSE, если скрыть.
void gp_tile_viewer_set_toolbar_visible( GpTileViewer *tile_viewer, gboolean toolbar_visible );



G_END_DECLS

#endif /* __GP_TILE_VIEWER_H__ */

/// @}

