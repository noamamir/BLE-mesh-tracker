import { Pipe, PipeTransform } from '@angular/core';
import {ReceiverId} from "../models/receiver";

@Pipe({
  name: 'AliasNames'
})
export class AliasNamesPipe implements PipeTransform {

  transform(aliasMap: Readonly<Record<string, string>> | null, receiver:ReceiverId): string {
    if (aliasMap == null) {
      return receiver
    }
    return aliasMap[receiver] ?? receiver
  }

}
