/* 
 * gnc-tree-model-selection.h -- GtkTreeModel which supports a selectable column.
 * Copyright (C) 2003 Jan Arne Petersen
 * Author: Jan Arne Petersen <jpetersen@uni-bonn.de>
 */

#ifndef __GNC_TREE_MODEL_SELECTION_H
#define __GNC_TREE_MODEL_SELECTION_H

#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeviewcolumn.h>

G_BEGIN_DECLS

/* type macros */
#define GNC_TYPE_TREE_MODEL_SELECTION            (gnc_tree_model_selection_get_type ())
#define GNC_TREE_MODEL_SELECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNC_TYPE_TREE_MODEL_SELECTION, GncTreeModelSelection))
#define GNC_TREE_MODEL_SELECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GNC_TYPE_TREE_MODEL_SELECTION, GncTreeModelSelectionClass))
#define GNC_IS_TREE_MODEL_SELECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNC_TYPE_TREE_MODEL_SELECTION))
#define GNC_IS_TREE_MODEL_SELECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNC_TYPE_TREE_MODEL_SELECTION))
#define GNC_TREE_MODEL_SELECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GNC_TYPE_TREE_MODEL_SELECTION, GncTreeModelSelectionClass))

/* typedefs & structures */
typedef struct GncTreeModelSelectionPrivate GncTreeModelSelectionPrivate;

typedef struct {
	GObject parent;

	GncTreeModelSelectionPrivate *priv;

	int stamp;
} GncTreeModelSelection;

typedef struct {
	GObjectClass parent;
} GncTreeModelSelectionClass;

/* function prototypes */
GType              gnc_tree_model_selection_get_type                   (void);

GtkTreeModel      *gnc_tree_model_selection_new                        (GtkTreeModel *child_model);

GtkTreeModel      *gnc_tree_model_selection_get_model                  (GncTreeModelSelection *model);
void               gnc_tree_model_selection_convert_child_iter_to_iter (GncTreeModelSelection *model,
                                                                        GtkTreeIter *selection_iter,
									GtkTreeIter *child_iter);
void               gnc_tree_model_selection_convert_iter_to_child_iter (GncTreeModelSelection *model,
                                                                        GtkTreeIter *child_iter,
 									GtkTreeIter *selection_iter);

gint               gnc_tree_model_selection_get_selection_row          (GncTreeModelSelection *model);
GtkTreeViewColumn *gnc_tree_model_selection_create_tree_view_column    (GncTreeModelSelection *model,
									const gchar *title);

gboolean           gnc_tree_model_selection_is_selected                (GncTreeModelSelection *model,
                                                                        GtkTreeIter *iter);
void               gnc_tree_model_selection_set_selected               (GncTreeModelSelection *model,
                                                                        GtkTreeIter *iter,
									gboolean selected);

G_END_DECLS

#endif /* __GNC_TREE_MODEL_SELECTION_H */
