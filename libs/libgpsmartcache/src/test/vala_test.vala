public class Main : Object
{
  public static int main (string[] args)
  {
    // Две строки, что положим в кэш (первая -- с ошибкой).
    string str1 = "Beevis";
    string str2 = "Butt-head";

    Gp.SmartCacheAccessFunc condition;

    { //< Область видимости локальной переменной wanted -->

      // Ошибочная строка, которую будем в кэше искать.
      string wanted = "Beevis";

      // Анонимный метод, который находит строку wanted.
      condition = (data) =>
      {
        string str_in_cache = (string)data;

        if(str_in_cache == wanted) //< Прямо напрямую здесь используем локальную переменную.
        {
          print(@"$str_in_cache == $wanted\n");
          return true;
        }
        else
        {
          print(@"$str_in_cache != $wanted\n");
          return false;
        }
      };

    } //< Область видимости локальной переменной wanted <--

    // Анонимный метод, который чинит ошибку в строке wanted.
    Gp.SmartCacheAccessFunc modifier = (data) =>
    {
      data[2] = 'a';
      print("Fixing string in cache!\n");
      return true;
    };

    // Создаем кэш, ровно чтоб две строки влезло.
    var cache = new Gp.SmartCache();
    cache.set_size(str1.length + str2.length + 2);

    // Регистрируем группу, будем работать только с ней для простоты.
    uint group = cache.reg_group();

    // Положим две строки в кэш.
    // Индексы нам не важны, укажем просто разные -- ноль и один.
    cache.set(group, 0, (uint8[])str1.to_utf8());
    cache.set(group, 1, (uint8[])str2.to_utf8());

    // Запускаем фикс строки в кеше: он отработает как нужно.
    print("* Running modify!\n");
    cache.modify(group, condition, modifier);

    // Запускаем фикс строки в кеше: он не найдет битую строку в кэше.
    print("* Running modify!\n");
    cache.modify(group, condition, modifier);

    // Чистим кэш.
    print("* Cleaning!\n");
    cache.clean(group);

    // Запускаем фикс строки в кеше: он не найдет ничего в кэше.
    print("* Running modify!\n");
    cache.modify(group, condition, modifier);

    print("* Exiting!\n");
    return 0;
  }
}
