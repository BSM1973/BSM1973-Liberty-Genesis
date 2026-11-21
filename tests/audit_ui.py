import json,re,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
source=(root/'plugins/LibertyGenesis/src/LibertyGenesis.cpp').read_text(encoding='utf-8')
ui=json.loads((root/'plugins/LibertyGenesis/ui.json').read_text(encoding='utf-8'))
params=re.findall(r'addParam\(mpc::\w+Param\("[^"]+","([^"]+)"',source)
assert len(params)==len(set(params))==129
pages=ui['pages']
assert len(pages)==len({p['name'] for p in pages})==20
mapped=set()
for p in pages:
    assert 0<len(p['controls'])<=16,p['name']
    for c in p['controls']:
        if 'param' in c:
            assert c['param'] in params,(p['name'],c['param'])
            mapped.add(c['param'])
        for k in ('param','text'):
            if k in c:
                assert not any(x in c[k] for x in ('\ufffd','Ã','Â','â€')),c[k]
assert mapped==set(params),set(params)-mapped
print('UI structural audit: 20 pages, 129 parameters, complete mapping')
