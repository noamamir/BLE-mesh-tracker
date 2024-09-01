import {Injectable} from '@angular/core';
import {NgxIndexedDBService, WithID} from "ngx-indexed-db";
import {ReceiverId} from "../../models/receiver";
import {BehaviorSubject, Observable} from "rxjs";

@Injectable({
  providedIn: 'root'
})
export class ReceiverAliasService {
  aliasNames = new BehaviorSubject<Readonly<Record<ReceiverId, string>>>({})
  beaconAlias = new BehaviorSubject<Readonly<Record<ReceiverId, string>>>({})
  aliasStorage = "ReceiverAlias"
  BeaconAliasName = "BeaconAlias"
  constructor() {
    this.fetchAliasFromStorage()
  }

  private fetchAliasFromStorage() {
    const storedData = localStorage.getItem(this.aliasStorage)
    const parsedData =  storedData ? JSON.parse(storedData) : {};
    this.aliasNames.next(parsedData)

  }

  setAlias(name: string, alias: string): void {
    const currentAliases = this.aliasNames.value;
    const updatedAliases = { ...currentAliases, [name]: alias };

    localStorage.setItem(this.aliasStorage, JSON.stringify(updatedAliases));
    this.aliasNames.next(updatedAliases);
  }

}
