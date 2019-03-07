/*
 * GpTrunk is a thread-safe objects storage library.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * GpTrunk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpTrunk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpTrunk. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include "gp-core.h"

// Тестовый объект -->
  typedef struct _Foo
  {
    gint size;
    gchar name[16];
  }
  Foo;
  static volatile gint _total_foos_num = 0;

  Foo *foo_new(gint size, gpointer user_data)
  {
    #if GLIB_CHECK_VERSION(2, 30, 0)
      int n = g_atomic_int_add(&_total_foos_num, 1) + 1;
    #else
      int n = g_atomic_int_exchange_and_add(&_total_foos_num, 1) + 1;
    #endif

    Foo *rval = g_new0(Foo, 1);
    rval->size = size;
    g_snprintf(rval->name, sizeof(rval->name), "Foo #%d", n);

    g_print("Created: %s (size = %d, user_data = %p)\n", rval->name, rval->size, user_data);
    return rval;
  }

  void foo_free(Foo *foo)
  {
    g_assert(foo != NULL);

    g_atomic_int_add(&_total_foos_num, -1);
    g_print("Deleted: %s (size = %d)\n", foo->name, foo->size);
    g_free(foo);
  }

  Foo *foo_copy(Foo *foo)
  {
    g_assert_not_reached();
  }

  gint foo_num()
  {
    return g_atomic_int_get(&_total_foos_num);
  }

  void user_data_unref(gpointer data)
  {
    g_print("User data (%p) unref\n", data);
  }
// Тестовый объект <--


static volatile gint _created_threads_num = 0;

gpointer thread_func(GpTrunk *trunk)
{
  #if GLIB_CHECK_VERSION(2, 30, 0)
    gint thread_num = g_atomic_int_add(&_created_threads_num, 1) + 1;
  #else
    gint thread_num = g_atomic_int_exchange_and_add(&_created_threads_num, 1) + 1;
  #endif

  guint size = thread_num % 3;
  gchar thread_name[16];
  g_snprintf(thread_name, sizeof(thread_name), "Thread #%d", thread_num);
  g_print("Running %s, need size = %u\n", thread_name, size);

  // Собственно пример использования Trunk ->>
    // Создаем пустой указатель на GpTrunkGuard.
    GpTrunkGuard *guard = NULL;

    // Получаем новый экземпляр GpTrunkGuard и указатель на объект из хранилища.
    // Этот указатель привязан к времени жизни GpTrunkGuard!
    const Foo *foo = gp_trunk_access(trunk, &guard, size);
    g_assert(guard);
    g_assert(foo);

    // Повторные вызовы access с существующим GpTrunkGuard должны возвращать
    // указатель на тот же самый объект, что был получен при первом вызове access.
    g_assert(gp_trunk_access(trunk, &guard, size) == foo);
    g_assert(gp_trunk_access(trunk, &guard, size) == foo);

    g_usleep(G_USEC_PER_SEC / 10);

    // Не забудем уничтожить GpTrunkGuard.
    // Декструктор GpTrunkGuard вернет наш объект в хранилище GpTrunk.
    g_clear_object(&guard);
  // Собственно пример использования Trunk <<-

  return NULL;
}

int main(int argc, char **argv)
{
  #if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
  #endif

  // Собственно пример создания Trunk ->>
    GpTrunk *trunk = gp_trunk_new(
      -999, //< Не имеет значения.
      (GBoxedCopyFunc)foo_copy, //< Не должна вызываться, можно передать NULL (а иначе просто смысл в Trunk?).
      (GDestroyNotify)foo_free, //< Метод удаления объекта, нужен!
      (GpTrunkObjectCreateFunc)foo_new, //< Метод создания объекта, нужен!
      (void*)0xDEADBEEF, //< Указатель на user_data (если нужен), который будет передаваться в вызовы метода создания объекта.
      user_data_unref); //< Указатель на функцию (если нужен) уничтожения user_data.
  // Собственно пример создания Trunk <<-

  int i;
  GThread *threads[12] = { NULL };
  int num = G_N_ELEMENTS(threads);

  g_print("\tSequential run:\n");

  for(i = 0; i < num / 2; i++)
    thread_func(trunk);

  g_print("\tParallel run (current length = %d):\n", gp_trunk_length(trunk));

  for(; i < num; i++)
    threads[i] = g_thread_new(NULL, (GThreadFunc)thread_func, trunk);

  for(i = 0; i < num; i++)
    if(threads[i])
      g_thread_join(threads[i]);

  g_print("Before trunk destruction\n");
  g_assert_cmpuint(G_OBJECT(trunk)->ref_count, ==, 1);
  g_clear_object(&trunk);
  g_print("After trunk destruction\n");

  g_assert_cmpint(foo_num(), ==, 0);

  return 0;
}
