d = open("decode.html", encoding="utf-8").read()


def rep(old, new, tag):
    global d
    assert old in d, "MISS " + tag
    d = d.replace(old, new, 1)


# 1. Состояние правок протокола
rep("  cursorT: null, tMax: 0,",
    "  cursorT: null, tMax: 0,\n"
    "  manualViols: [],   // {start, text, m:true} - вручную добавленные\n"
    "  deletedAuto: [],   // ключи удалённых автоматических", "state")

# 2. Единый список: авто (не удалённые) + ручные
rep("function viol(start, end, text){",
    "function violKey(v){ return v.start.toFixed(2) + \"|\" + v.text; }\n"
    "function getViolations(){\n"
    "  const auto = computeViolations()\n"
    "    .filter(v => !state.deletedAuto.includes(violKey(v)));\n"
    "  return auto.concat(state.manualViols)\n"
    "    .sort((a, b) => a.start - b.start);\n"
    "}\n"
    "function saveViols(){\n"
    "  try {\n"
    "    localStorage.setItem(\"kr_\" + state.serial, JSON.stringify({\n"
    "      manual: state.manualViols, deleted: state.deletedAuto}));\n"
    "  } catch(e) {}\n"
    "}\n"
    "function loadViols(){\n"
    "  try {\n"
    "    const s = localStorage.getItem(\"kr_\" + state.serial);\n"
    "    if (s){\n"
    "      const o = JSON.parse(s);\n"
    "      state.manualViols = (o.manual || []).map(v =>\n"
    "        ({start: v.start, end: v.start, text: v.text, m: true}));\n"
    "      state.deletedAuto = o.deleted || [];\n"
    "    }\n"
    "  } catch(e) {}\n"
    "}\n"
    "function viol(start, end, text){", "getv")

# 3. Редактируемая панель нарушений
old_render = d[d.index("function renderViolations(){"):]
old_render = old_render[:old_render.index("}\n\nfunction renderButtons")]
new_render = '''function renderViolations(){
  const el = document.getElementById("violations");
  const v = getViolations();
  if(v.length === 0){
    el.innerHTML = "<div class='small'>— нарушений не выявлено</div>";
  } else {
    el.innerHTML = "";
    v.forEach((x, i) => {
      const div = document.createElement("div");
      div.className = "violation";
      div.textContent = "№" + (i + 1) + "  " + hhmmss(x.start) +
        "  " + kmPcM(coordAt(x.start)) + "  " + x.text +
        (x.m ? "  [вручную]" : "") +
        (x.end > x.start
          ? "  (" + (x.end - x.start).toFixed(0) + " с)" : "");

      const del = document.createElement("span");
      del.textContent = " [×]";
      del.style.color = "#ff5252";
      del.style.cursor = "pointer";
      del.title = "Убрать из протокола";
      del.onclick = (e) => {
        e.stopPropagation();
        if (x.m)
          state.manualViols = state.manualViols.filter(m => m !== x);
        else
          state.deletedAuto.push(violKey(x));
        saveViols();
        renderViolations();
      };
      div.appendChild(del);

      div.onclick = () => { state.cursorT = x.start; renderAll(); };
      el.appendChild(div);
    });
  }

  // Форма ручного добавления (время берётся с курсора)
  const form = document.createElement("div");
  form.style.marginTop = "6px";
  form.innerHTML =
    "<input id='mv-text' placeholder='текст нарушения' " +
    "style='width:55%;background:#111;color:#c8c8c8;" +
    "border:1px solid #3c3c40;padding:2px 5px'> " +
    "<button id='mv-add' class='x' style='background:#0e5a80;" +
    "color:#fff;border:1px solid #1e7fb0;padding:2px 10px;" +
    "cursor:pointer'>+ Добавить</button>" +
    "<div class='small'>добавится на время курсора</div>";
  el.appendChild(form);
  document.getElementById("mv-add").onclick = () => {
    const t = prompt("Время нарушения, сек (пусто - позиция курсора):",
      Math.round(state.cursorT === null ? state.tMax : state.cursorT));
    if (t === null) return;
    const start = parseFloat(t);
    if (isNaN(start) || start < 0) { alert("Некорректное время"); return; }
    const txt = prompt("Формулировка нарушения:");
    if (!txt) return;
    state.manualViols.push({start, end: start, text: txt, m: true});
    saveViols();
    renderViolations();
  };
}'''
d = d.replace(old_render, new_render, 1)

# 4. Печать: единый список + пометка ручных
rep("function printProtocol(){\n  const v = computeViolations();",
    "function printProtocol(){\n  const v = getViolations();", "print1")
rep("'<td>' + esc(x.text) + '</td>'",
    "'<td>' + esc(x.text) + (x.m ? ' [вручную]' : '') + '</td>'", "print2")
rep("'<h2>Выявленные нарушения (' + v.length + ')</h2>' +",
    "'<h2>Выявленные нарушения (' + v.length +\n"
    "      (state.deletedAuto.length ?\n"
    "        ', автоматически удалено: ' + state.deletedAuto.length\n"
    "        : '') + ')</h2>' +", "print3")

# 5. Загрузка/сброс правок при открытии файла
rep("    renderMeta();",
    "    state.manualViols = [];\n"
    "    state.deletedAuto = [];\n"
    "    loadViols();\n"
    "    renderMeta();", "load")

open("decode.html", "w", encoding="utf-8").write(d)
print("edit ok, braces", d.count("{") - d.count("}"))
