import {NgxIndexedDBModule, DBConfig} from 'ngx-indexed-db';

export const dbConfig: DBConfig = {
  name: 'MyDb',
  version: 1,
  objectStoresMeta: [
    {
      store: 'receivers',
      storeConfig: {keyPath: 'UUID', autoIncrement: false},
      storeSchema: [
        {name: 'alias', keypath: 'alias', options: {unique: true}},
      ]
    }
  ]
};
